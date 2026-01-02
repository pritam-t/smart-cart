from barcode import Code128
from barcode.writer import ImageWriter

# Barcode value (can be any length for CODE-128)
barcode_value = "8901030891234"

# Generate barcode
barcode = Code128(barcode_value, writer=ImageWriter())

# Save barcode image
barcode.save("code128_barcode")

print("CODE-128 barcode generated successfully!")
