#include "sample_extension.h"

#include <random>
#include <string.h>
#include <dart_native_api.h>

// Forward declaration of ResolveName function.
Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool *auto_setup_scope);

// The name of the initialization function is the extension name followed
// by _Init.
DART_EXPORT Dart_Handle sample_extension_Init(Dart_Handle parent_library) {
    if (Dart_IsError(parent_library)) return parent_library;

    Dart_Handle result_code =
            Dart_SetNativeResolver(parent_library, ResolveName, NULL);
    if (Dart_IsError(result_code)) return result_code;

    return Dart_Null();
}

Dart_Handle HandleError(Dart_Handle handle) {
    if (Dart_IsError(handle)) Dart_PropagateError(handle);
    return handle;
}

// Native functions get their arguments in a Dart_NativeArguments structure
// and return their results with Dart_SetReturnValue.
void SystemRand(Dart_NativeArguments arguments) {
    Dart_Handle result = HandleError(Dart_NewInteger(rand()));
    Dart_SetReturnValue(arguments, result);
}

void SystemSrand(Dart_NativeArguments arguments) {
    bool success = false;
    Dart_Handle seed_object =
            HandleError(Dart_GetNativeArgument(arguments, 0));
    if (Dart_IsInteger(seed_object)) {
        bool fits;
        HandleError(Dart_IntegerFitsIntoInt64(seed_object, &fits));
        if (fits) {
            int64_t seed;
            HandleError(Dart_IntegerToInt64(seed_object, &seed));
            srand(static_cast<unsigned>(seed));
            success = true;
        }
    }
    Dart_SetReturnValue(arguments, HandleError(Dart_NewBoolean(success)));
}

uint8_t *random_array(int seed, int length) {
    if (length <= 0 || length > 10000000) return NULL;

    uint8_t *values = reinterpret_cast<uint8_t *>(malloc(length));
    if (NULL == values) return NULL;

    srand(seed);
    for (int i = 0; i < length; ++i) {
        values[i] = rand() % 256;
    }
    return values;
}

void wrappedRandomArray(Dart_Port dest_port_id,
                        Dart_CObject *message) {
    if (message->type == Dart_CObject_kArray &&
        3 == message->value.as_array.length) {
        // Use .as_array and .as_int32 to access the data in the Dart_CObject.
        Dart_CObject *param0 = message->value.as_array.values[0];
        Dart_CObject *param1 = message->value.as_array.values[1];
        Dart_CObject *param2 = message->value.as_array.values[2];
        if (param0->type == Dart_CObject_kSendPort) {
            Dart_Port reply_port_id = param0->value.as_send_port.id;

            if (param1->type == Dart_CObject_kInt32 &&
                param2->type == Dart_CObject_kInt32) {
                int seed = param1->value.as_int32;
                int length = param2->value.as_int32;

                uint8_t *values = random_array(seed, length);

                if (values != NULL) {
                    Dart_CObject result;
                    result.type = Dart_CObject_kTypedData;
                    result.value.as_typed_data.type = Dart_TypedData_kUint8;
                    result.value.as_typed_data.length = length;
                    result.value.as_typed_data.values = values;
                    Dart_PostCObject(reply_port_id, &result);
                    free(values);
                    // It is OK that result is destroyed when function exits.
                    // Dart_PostCObject has copied its data.
                    return;
                }
            }

            Dart_CObject result;
            result.type = Dart_CObject_kNull;
            Dart_PostCObject(reply_port_id, &result);
        }
    }
    // TODO we don't have access to a reply port out in this scope where even
    // the 0th parameter proved invalid... I guess we just won't reply...
}

void RandomArray_ServicePort(Dart_NativeArguments arguments) {
    Dart_SetReturnValue(arguments, Dart_Null());
    Dart_Port service_port =
            Dart_NewNativePort("RandomArrayService", wrappedRandomArray, true);
    if (service_port != ILLEGAL_PORT) {
        Dart_Handle send_port = Dart_NewSendPort(service_port);
        Dart_SetReturnValue(arguments, send_port);
    }
}

Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool *auto_setup_scope) {
// If we fail, we return NULL, and Dart throws an exception.
    if (!Dart_IsString(name)) return NULL;
    Dart_NativeFunction result = NULL;
    const char *cname;
    HandleError(Dart_StringToCString(name, &cname));

    if (strcmp("SystemRand", cname) == 0) result = SystemRand;
    if (strcmp("SystemSrand", cname) == 0) result = SystemSrand;
    if (strcmp("RandomArray_ServicePort", cname) == 0) result = RandomArray_ServicePort;
    return result;
}
