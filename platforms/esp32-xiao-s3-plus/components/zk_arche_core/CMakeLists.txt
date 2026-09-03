set(ZK_ARCHE_C_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../c")

file(GLOB ZK_ARCHE_CRYPTO_SRCS "${ZK_ARCHE_C_ROOT}/src/crypto/*.c")
file(GLOB ZK_ARCHE_WIRE_SRCS   "${ZK_ARCHE_C_ROOT}/src/wire/*.c")
file(GLOB ZK_ARCHE_PROTO_SRCS  "${ZK_ARCHE_C_ROOT}/src/proto/*.c")

idf_component_register(
    SRCS
        ${ZK_ARCHE_CRYPTO_SRCS}
        ${ZK_ARCHE_WIRE_SRCS}
        ${ZK_ARCHE_PROTO_SRCS}
    INCLUDE_DIRS
        "${ZK_ARCHE_C_ROOT}/include"
    REQUIRES
        libsodium
)

# Treat the common core as portable C11. Do not redefine protocol constants
# or primitives here; target-specific changes belong behind explicit adapters.
target_compile_features(${COMPONENT_LIB} PRIVATE c_std_11)
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wall -Wextra -Wpedantic
    -fno-strict-aliasing
)
