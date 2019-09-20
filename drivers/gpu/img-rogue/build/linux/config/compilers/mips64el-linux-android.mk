# 64-bit MIPS R6 little-endian compiler
TARGET_PRIMARY_ARCH   := target_mips64r6el
ifeq ($(MULTIARCH),1)
# For backwards ABI compatibility, r2 (not r6) must be used for the other
# architecture
TARGET_SECONDARY_ARCH := target_mips32r2el
endif
