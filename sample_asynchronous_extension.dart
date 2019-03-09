library sample_asynchronous_extension;

import 'dart:isolate';

import 'dart-ext:sample_extension';

// A class caches the native port used to call an asynchronous extension.
class RandomArray {
  static SendPort _port;

  void randomArray(int seed, int length, void callback(List result)) {
    ReceivePort receive_port = ReceivePort();

    var args = new List(3);
    args[0] = receive_port.sendPort;
    args[1] = seed;
    args[2] = length;

    _servicePort.send(args);
    receive_port.first.then((result) {
      if (result != null) {
        callback(result);
      } else {
        throw new Exception("Random array creation failed");
      }
      receive_port.close();
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