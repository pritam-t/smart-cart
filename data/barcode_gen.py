from barcode import Code128
from barcode.writer import ImageWriter
import os

products = [
    ("HeadPhones", "890000000001"),
    ("Bread", "890000000002"),
    ("Chocolate", "890000000003"),
    ("Juice", "890000000004"),
    ("Sugar", "890000000005")
]

# Save inside the smart-cart/barcodes folder
output_folder = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "barcodes"
)

# Create the folder if it doesn't exist
os.makedirs(output_folder, exist_ok=True)

for name, barcode_value in products:
    barcode = Code128(barcode_value, writer=ImageWriter())

    filename = os.path.join(
        output_folder,
        f"{name}_{barcode_value}"
    )

    barcode.save(filename)

    print(f"Generated: {name}_{barcode_value}.png")

print("\nAll 5 barcodes generated successfully!")
print(f"Saved in: {output_folder}")