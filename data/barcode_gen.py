from barcode import EAN13
from barcode.writer import ImageWriter
import os

products = [
    ("Amul_Milk", "7143265017727"),
    ("Bread", "6161235907107"),
    ("Chocolate", "7032572402817"),
    ("Yoga_Bar", "1433116676521"),
    ("Mango_Juice", "8811882639064"),
    ("Potato_Chips", "8342634755560"),
    ("Deodorant", "9033474318990"),
    ("Face_Wash", "3360540974160"),
    ("T_Shirt", "5695264743432"),
    ("Soap", "4294054344562"),
    ("Shoes", "1286480389555"),
    ("Biscuit", "6668846453240"),
    ("Snacks", "1362573641042"),
    ("Juice", "3122418982833"),
    ("Sugar", "7665125659873"),
    ("HeadPhones", "8507770407289")
]

# Save inside smart-cart/barcodes
output_folder = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "barcodes"
)

os.makedirs(output_folder, exist_ok=True)

for name, barcode_value in products:

    # EAN-13 needs the first 12 digits.
    # The library calculates the 13th check digit.
    barcode = EAN13(
        barcode_value[:12],
        writer=ImageWriter()
    )

    filename = os.path.join(
        output_folder,
        f"{name}_{barcode_value}"
    )

    barcode.save(filename)

    print(f"Generated: {name} -> {barcode_value}")

print("\nAll original product barcodes generated successfully!")
print(f"Saved in: {output_folder}")