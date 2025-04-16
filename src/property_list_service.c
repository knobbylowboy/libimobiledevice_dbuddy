/*
 * property_list_service.c
 * PropertyList service implementation.
 *
 * Copyright (c) 2010 Nikias Bassen. All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "property_list_service.h"
#include "common/debug.h"
#include "endianness.h"

/**
 * Convert a service_error_t value to a property_list_service_error_t value.
 * Used internally to get correct error codes.
 *
 * @param err A service_error_t error code
 *
 * @return A matching property_list_service_error_t error code,
 *     PROPERTY_LIST_SERVICE_E_UNKNOWN_ERROR otherwise.
 */
static property_list_service_error_t service_to_property_list_service_error(service_error_t err)
{
	switch (err) {
		case SERVICE_E_SUCCESS:
			return PROPERTY_LIST_SERVICE_E_SUCCESS;
		case SERVICE_E_INVALID_ARG:
			return PROPERTY_LIST_SERVICE_E_INVALID_ARG;
		case SERVICE_E_MUX_ERROR:
			return PROPERTY_LIST_SERVICE_E_MUX_ERROR;
		case SERVICE_E_SSL_ERROR:
			return PROPERTY_LIST_SERVICE_E_SSL_ERROR;
		case SERVICE_E_NOT_ENOUGH_DATA:
			return PROPERTY_LIST_SERVICE_E_NOT_ENOUGH_DATA;
		case SERVICE_E_TIMEOUT:
			return PROPERTY_LIST_SERVICE_E_RECEIVE_TIMEOUT;
		default:
			break;
	}
	return PROPERTY_LIST_SERVICE_E_UNKNOWN_ERROR;
}

property_list_service_error_t property_list_service_client_new(idevice_t device, lockdownd_service_descriptor_t service, property_list_service_client_t *client)
{
	if (!device || !service || service->port == 0 || !client || *client)
		return PROPERTY_LIST_SERVICE_E_INVALID_ARG;

	service_client_t parent = NULL;
	service_error_t rerr = service_client_new(device, service, &parent);
	if (rerr != SERVICE_E_SUCCESS) {
		return service_to_property_list_service_error(rerr);
	}

	/* create client object */
	property_list_service_client_t client_loc = (property_list_service_client_t)malloc(sizeof(struct property_list_service_client_private));
	client_loc->parent = parent;

	/* all done, return success */
	*client = client_loc;
	return PROPERTY_LIST_SERVICE_E_SUCCESS;
}

property_list_service_error_t property_list_service_client_free(property_list_service_client_t client)
{
	if (!client)
		return PROPERTY_LIST_SERVICE_E_INVALID_ARG;

	property_list_service_error_t err = service_to_property_list_service_error(service_client_free(client->parent));

	free(client);
	client = NULL;

	return err;
}

/**
 * Sends a plist using the given property list service client.
 * Internally used generic plist send function.
 *
 * @param client The property list service client to use for sending.
 * @param plist plist to send
 * @param binary 1 = send binary plist, 0 = send xml plist
 *
 * @return PROPERTY_LIST_SERVICE_E_SUCCESS on success,
 *      PROPERTY_LIST_SERVICE_E_INVALID_ARG when one or more parameters are
 *      invalid, PROPERTY_LIST_SERVICE_E_PLIST_ERROR when dict is not a valid
 *      plist, PROPERTY_LIST_SERVICE_E_MUX_ERROR when a communication error
 *      occurs, or PROPERTY_LIST_SERVICE_E_UNKNOWN_ERROR when an unspecified
 *      error occurs.
 */
static property_list_service_error_t internal_plist_send(property_list_service_client_t client, plist_t plist, int binary)
{
	property_list_service_error_t res = PROPERTY_LIST_SERVICE_E_UNKNOWN_ERROR;
	char *content = NULL;
	uint32_t length = 0;
	uint32_t nlen = 0;
	uint32_t bytes = 0;

	if (!client || (client && !client->parent) || !plist) {
		return PROPERTY_LIST_SERVICE_E_INVALID_ARG;
	}

	if (binary) {
		plist_to_bin(plist, &content, &length);
	} else {
		plist_to_xml(plist, &content, &length);
	}

	if (!content || length == 0) {
		return PROPERTY_LIST_SERVICE_E_PLIST_ERROR;
	}

	nlen = htobe32(length);
	debug_info("sending %d bytes", length);
	service_send(client->parent, (const char*)&nlen, sizeof(nlen), &bytes);
	if (bytes == sizeof(nlen)) {
		service_send(client->parent, content, length, &bytes);
		if (bytes > 0) {
			debug_info("sent %d bytes", bytes);
			debug_plist(plist);
			if (bytes == length) {
				res = PROPERTY_LIST_SERVICE_E_SUCCESS;
			} else {
				debug_info("ERROR: Could not send all data (%d of %d)!", bytes, length);
			}
		}
	}
	if (bytes <= 0) {
		debug_info("ERROR: sending to device failed.");
		res = PROPERTY_LIST_SERVICE_E_MUX_ERROR;
	}

	free(content);
	return res;
}

property_list_service_error_t property_list_service_send_xml_plist(property_list_service_client_t client, plist_t plist)
{
	return internal_plist_send(client, plist, 0);
}

property_list_service_error_t property_list_service_send_binary_plist(property_list_service_client_t client, plist_t plist)
{
	return internal_plist_send(client, plist, 1);
}

/**
 * Receives a plist using the given property list service client.
 * Internally used generic plist receive function.
 *
 * @param client The property list service client to use for receiving
 * @param plist pointer to a plist_t that will point to the received plist
 *      upon successful return
 * @param timeout Maximum time in milliseconds to wait for data.
 *
 * @return PROPERTY_LIST_SERVICE_E_SUCCESS on success,
 *      PROPERTY_LIST_SERVICE_E_INVALID_ARG when client or *plist is NULL,
 *      PROPERTY_LIST_SERVICE_E_NOT_ENOUGH_DATA when not enough data
 *      received, PROPERTY_LIST_SERVICE_E_RECEIVE_TIMEOUT when the connection times out,
 *      PROPERTY_LIST_SERVICE_E_PLIST_ERROR when the received data cannot be
 *      converted to a plist, PROPERTY_LIST_SERVICE_E_MUX_ERROR when a
 *      communication error occurs, or PROPERTY_LIST_SERVICE_E_UNKNOWN_ERROR
 *      when an unspecified error occurs.
 */
static property_list_service_error_t internal_plist_receive_timeout(property_list_service_client_t client, plist_t *plist, unsigned int timeout)
{
#ifdef DEBUG_MOBILEBACKUP2
	fprintf(stderr, "====== PLS: internal_plist_receive_timeout called with timeout %d ======\n", timeout);
	fflush(stderr);
#endif
	property_list_service_error_t res = PROPERTY_LIST_SERVICE_E_UNKNOWN_ERROR;
	uint32_t pktlen = 0;
	uint32_t bytes = 0;

	if (!client || (client && !client->parent) || !plist) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: ERROR - Invalid arguments ======\n");
		fflush(stderr);
#endif
		return PROPERTY_LIST_SERVICE_E_INVALID_ARG;
	}

	*plist = NULL;
#ifdef DEBUG_MOBILEBACKUP2
	fprintf(stderr, "====== PLS: Calling service_receive_with_timeout ======\n");
	fflush(stderr);
	/* When debugging, add a small delay before each receive to give system time */
	usleep(100000); /* 100ms */
#endif
	service_error_t serr = service_receive_with_timeout(client->parent, (char*)&pktlen, sizeof(pktlen), &bytes, timeout);
	if (serr != SERVICE_E_SUCCESS) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: Initial read failed with error %d ======\n", serr);
		fflush(stderr);
#endif
		debug_info("initial read failed!");
		return service_to_property_list_service_error(serr);
	}

	if (bytes == 0) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: No data received ======\n");
		fflush(stderr);
#endif
		debug_info("no data received");
		return PROPERTY_LIST_SERVICE_E_NOT_ENOUGH_DATA;
	} else if (bytes < sizeof(pktlen)) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: Not enough data received for packet length (%d of %d bytes) ======\n", bytes, sizeof(pktlen));
		fflush(stderr);
#endif
		debug_info("not enough data for packet length");
		return PROPERTY_LIST_SERVICE_E_NOT_ENOUGH_DATA;
	}

	pktlen = be32toh(pktlen);
	if (pktlen < 1) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: Invalid packet length received: %d ======\n", pktlen);
		fflush(stderr);
#endif
		debug_info("invalid packet length");
		return PROPERTY_LIST_SERVICE_E_UNKNOWN_ERROR;
	}

	char *content = (char*)malloc(pktlen);
	if (!content) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: ERROR - Out of memory when allocating %d bytes ======\n", pktlen);
		fflush(stderr);
#endif
		return PROPERTY_LIST_SERVICE_E_UNKNOWN_ERROR;
	}

#ifdef DEBUG_MOBILEBACKUP2
	fprintf(stderr, "====== PLS: Attempting to receive %d bytes of data ======\n", pktlen);
	fflush(stderr);
	/* Add a small delay before receive when debugging */
	usleep(100000); /* 100ms */
#endif
	serr = service_receive_with_timeout(client->parent, content, pktlen, &bytes, timeout);
	if (serr != SERVICE_E_SUCCESS) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: ERROR - Failed to receive data, error %d ======\n", serr);
		fflush(stderr);
#endif
		debug_info("service_receive_with_timeout failed: %d", serr);
		free(content);
		return service_to_property_list_service_error(serr);
	}

	if (bytes != pktlen) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: ERROR - Received incomplete data (%d of %d bytes) ======\n", bytes, pktlen);
		fflush(stderr);
#endif
		free(content);
		return PROPERTY_LIST_SERVICE_E_NOT_ENOUGH_DATA;
	}

#ifdef DEBUG_MOBILEBACKUP2
	fprintf(stderr, "====== PLS: Successfully received %d bytes of data, attempting to parse ======\n", bytes);
	fflush(stderr);
#endif
	plist_from_xml(content, bytes, plist);

	if (*plist) {
#ifdef DEBUG_MOBILEBACKUP2
		fprintf(stderr, "====== PLS: Successfully parsed plist from XML ======\n");
		fflush(stderr);
#endif
		debug_info("successfully parsed plist");
		debug_plist(*plist);
		res = PROPERTY_LIST_SERVICE_E_SUCCESS;
	} else {
		debug_info("parsing XML to plist failed!");
		/* try to parse binary plist */
		plist_from_bin(content, bytes, plist);
		if (*plist) {
#ifdef DEBUG_MOBILEBACKUP2
			fprintf(stderr, "====== PLS: Successfully parsed plist from binary ======\n");
			fflush(stderr);
#endif
			debug_info("successfully parsed binary plist");
			debug_plist(*plist);
			res = PROPERTY_LIST_SERVICE_E_SUCCESS;
		} else {
#ifdef DEBUG_MOBILEBACKUP2
			fprintf(stderr, "====== PLS: ERROR - Failed to parse plist data (not XML or binary) ======\n");
			fflush(stderr);
#endif
			debug_info("failed to parse binary plist too!");
			res = PROPERTY_LIST_SERVICE_E_PLIST_ERROR;
		}
	}

	free(content);
	return res;
}

property_list_service_error_t property_list_service_receive_plist_with_timeout(property_list_service_client_t client, plist_t *plist, unsigned int timeout)
{
	return internal_plist_receive_timeout(client, plist, timeout);
}

property_list_service_error_t property_list_service_receive_plist(property_list_service_client_t client, plist_t *plist)
{
#ifdef DEBUG_MOBILEBACKUP2
	/* Use a much longer timeout when debugging to allow time for debugger stepping */
	return internal_plist_receive_timeout(client, plist, 120000); /* 2 minutes timeout when debugging */
#else
	return internal_plist_receive_timeout(client, plist, 30000);
#endif
}

property_list_service_error_t property_list_service_enable_ssl(property_list_service_client_t client)
{
	if (!client || !client->parent)
		return PROPERTY_LIST_SERVICE_E_INVALID_ARG;
	return service_to_property_list_service_error(service_enable_ssl(client->parent));
}

property_list_service_error_t property_list_service_disable_ssl(property_list_service_client_t client)
{
	if (!client || !client->parent)
		return PROPERTY_LIST_SERVICE_E_INVALID_ARG;
	return service_to_property_list_service_error(service_disable_ssl(client->parent));
}

property_list_service_error_t property_list_service_get_service_client(property_list_service_client_t client, service_client_t *service_client)
{
	if (!client || !client->parent || !service_client)
		return PROPERTY_LIST_SERVICE_E_INVALID_ARG;
	*service_client = client->parent;
	return PROPERTY_LIST_SERVICE_E_SUCCESS;
}
