library sample_asynchronous_extension;

import 'dart:isolate';

import 'dart-ext:sample_extension';

// A class caches the native port used to call an asynchronous extension.
class RandomArray {
  static SendPort _port;

  void randomArray(int seed, int length, void callback(List result)) {
    ReceivePort receivePort = ReceivePort();

    var args = new List(3);
    args[0] = receivePort.sendPort;
    args[1] = seed;
    args[2] = length;

    _servicePort.send(args);
    // I wanted to use receive_port.single(...), since I know I'll only send a
    // single message to this port, but it looks like .single(...) waits for
    // port closure to ensure only one item was ever sent before passing that
    // item to the callback. I can't figure out how to close the port from the
    // C++ side, so for now I'm using .first(...) and closing the port inside
    // the callback.
    receivePort.first.then((result) {
      if (result != null) {
        callback(result);
      } else {
        throw new Exception("Random array creation failed");
      }
      receivePort.close();
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