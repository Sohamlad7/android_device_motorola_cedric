$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

# Screen density
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xhdpi

# Boot animation
TARGET_SCREEN_HEIGHT := 1280
TARGET_SCREEN_WIDTH := 720

# Charger
PRODUCT_PACKAGES += \
    charger_res_images \
    charger

# Device identifier. This must come after all inclusions
PRODUCT_DEVICE := cedric
PRODUCT_NAME := full_cedric
PRODUCT_BRAND := Motorola
#PRODUCT_MODEL := G5
PRODUCT_MANUFACTURER := Motorola

PRODUCT_SYSTEM_PROPERTY_BLACKLIST += \
ro.product.model
