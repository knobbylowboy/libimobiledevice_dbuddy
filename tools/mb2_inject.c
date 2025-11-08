#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <plist/plist.h>
#include <libimobiledevice/mobilebackup2.h>

static mobilebackup2_error_t (*orig_mobilebackup2_send_request)(mobilebackup2_client_t, const char*, const char*, const char*, plist_t) = NULL;

static void add_domain_whitelist(plist_t *options_ptr)
{
	const char *domain_env = getenv("DBUDDY_DOMAIN_LIST");
	if (!domain_env || !*domain_env) {
		return;
	}

	const char *key_env = getenv("DBUDDY_DOMAIN_KEY");
	const char *key_name = (key_env && *key_env) ? key_env : "BackupOnlyDomains";

	plist_t options = *options_ptr;
	if (!options) {
		options = plist_new_dict();
		*options_ptr = options;
	}

	plist_t whitelist = plist_new_array();

	char *mutable = strdup(domain_env);
	if (!mutable) {
		plist_free(whitelist);
		return;
	}
	char *saveptr = NULL;
	char *token = strtok_r(mutable, ",", &saveptr);
	while (token) {
		while (*token == ' ' || *token == '\t') token++;
		char *end = token + strlen(token);
		while (end > token && (end[-1] == ' ' || end[-1] == '\t')) {
			end--;
		}
		*end = '\0';
		if (*token) {
			plist_array_append_item(whitelist, plist_new_string(token));
		}
		token = strtok_r(NULL, ",", &saveptr);
	}
	free(mutable);

	if (plist_array_get_size(whitelist) == 0) {
		plist_free(whitelist);
		return;
	}

	plist_dict_set_item(options, key_name, whitelist);
	fprintf(stderr, "[mb2_inject] Injected domain whitelist key '%s' with %u entries\n", key_name, plist_array_get_size(whitelist));
}

mobilebackup2_error_t mobilebackup2_send_request(mobilebackup2_client_t client, const char *request, const char *target_identifier, const char *source_identifier, plist_t options)
{
	if (!orig_mobilebackup2_send_request) {
		orig_mobilebackup2_send_request = (mobilebackup2_error_t (*)(mobilebackup2_client_t, const char*, const char*, const char*, plist_t))dlsym(RTLD_NEXT, "mobilebackup2_send_request");
		if (!orig_mobilebackup2_send_request) {
			fprintf(stderr, "[mb2_inject] Failed to resolve original mobilebackup2_send_request\n");
			return MOBILEBACKUP2_E_UNKNOWN_ERROR;
		}
	}

	plist_t opts_to_use = options;
	int created_opts = 0;

	if (request && strcmp(request, "Backup") == 0) {
		if (!opts_to_use) {
			opts_to_use = plist_new_dict();
			created_opts = 1;
		}
		add_domain_whitelist(&opts_to_use);
	}

	mobilebackup2_error_t ret = orig_mobilebackup2_send_request(client, request, target_identifier, source_identifier, opts_to_use);

	if (created_opts && opts_to_use) {
		plist_free(opts_to_use);
	}

	return ret;
}

