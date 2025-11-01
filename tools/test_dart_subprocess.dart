import 'dart:io';
import 'dart:async';
import 'dart:convert';

void main() async {
  print('Testing mock_idevicebackup2_win.exe buffering behavior in Dart subprocess...\n');
  
  // Test 1: Quick encryption command (should return immediately)
  print('=== Test 1: Encryption command (immediate return) ===');
  await testSubprocess(['mock_idevicebackup2_win.exe', '--info', 'encryption off', '--scenario', '1']);
  
  print('\n=== Test 2: Progress simulation (real-time output) ===');
  await testSubprocess(['mock_idevicebackup2_win.exe', '--time', '10']);
  
  print('\n=== Test 3: With delay (testing buffering) ===');
  await testSubprocess(['mock_idevicebackup2_win.exe', '--time', '5', '--delay', '2']);
}

Future<void> testSubprocess(List<String> args) async {
  print('Running: ${args.join(' ')}');
  print('Output:');
  print('-' * 50);
  
  try {
    final process = await Process.start(args[0], args.skip(1).toList());
    
    // Listen to stdout with real-time output
    process.stdout.transform(utf8.decoder).listen((data) {
      stdout.write(data); // Write immediately without buffering
    });
    
    // Listen to stderr
    process.stderr.transform(utf8.decoder).listen((data) {
      stderr.write(data); // Write immediately without buffering
    });
    
    // Wait for process to complete
    final exitCode = await process.exitCode;
    print('\n' + '-' * 50);
    print('Process completed with exit code: $exitCode');
    
  } catch (e) {
    print('Error running subprocess: $e');
  }
} 