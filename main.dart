import "sample_synchronous_extension.dart";
import "sample_asynchronous_extension.dart";

main() {
  systemSrand(5);
  print(systemRand());

  RandomArray r = RandomArray();
  r.randomArray(0, 10, (List result) {
    print(result);
  });
}