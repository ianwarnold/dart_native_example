library sample_asynchronous_extension;

import 'dart:isolate';

import 'dart-ext:sample_extension';

// A class caches the native port used to call an asynchronous extension.
class RandomArray {
  static SendPort _port;

  void randomArray(int seed, int length, void callback(List result)) {
    // To instantiate a new ReceivePort per request is at least no worse than
    // the original example, since that's what it was doing originally:
    // https://github.com/dart-lang/sdk/blob/3481c5661a43d6bf2d1766fa8bb4e1dbc79025ff/runtime/lib/isolate.dart#L105-L109
    ReceivePort receivePort = ReceivePort();

    var args = new List(3);
    args[0] = receivePort.sendPort;
    args[1] = seed;
    args[2] = length;

    _servicePort.send(args);

    // https://github.com/dart-lang/sdk/blob/3481c5661a43d6bf2d1766fa8bb4e1dbc79025ff/runtime/lib/isolate.dart#L74-L79
    // it seems like receive(...) was listen(...) from before ReceivePort was a stream.
    // https://github.com/dart-lang/sdk/commit/6a72655d1bba6f1a9cd4f6c55d9e09e890a8c3bb#diff-4e3d60ef4549b538efb7a736de5d7aabR134
    // https://github.com/dart-lang/sdk/commit/6a72655d1bba6f1a9cd4f6c55d9e09e890a8c3bb#diff-4e3d60ef4549b538efb7a736de5d7aabL150
    // https://github.com/dart-lang/sdk/commit/6a72655d1bba6f1a9cd4f6c55d9e09e890a8c3bb#diff-4e3d60ef4549b538efb7a736de5d7aabR162
    receivePort.first.then((result) {
      receivePort.close();
      if (result != null) {
        callback(result);
      } else {
        throw new Exception("Random array creation failed");
      }
    });
  }

  SendPort get _servicePort {
    if (_port == null) {
      _port = _newServicePort();
    }
    return _port;
  }

  SendPort _newServicePort() native "RandomArray_ServicePort";
}