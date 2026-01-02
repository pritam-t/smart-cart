/* ===============================
   BARCODE SCANNER LOGIC (FINAL)
================================ */

let scannerActive = false;
let scanLocked = false;

/* ===============================
   EAN-13 VALIDATION
================================ */
function isValidEAN13(code) {
  if (!/^\d{13}$/.test(code)) return false;

  let sum = 0;
  for (let i = 0; i < 12; i++) {
    sum += parseInt(code[i]) * (i % 2 === 0 ? 1 : 3);
  }

  const checksum = (10 - (sum % 10)) % 10;
  return checksum === parseInt(code[12]);
}

/* ===============================
   START CAMERA SCANNER
================================ */
function startScanner() {
  if (scannerActive) return;

  const scannerElement = document.getElementById("scanner");
  if (!scannerElement) {
    console.error("Scanner container not found");
    return;
  }

  Quagga.init(
    {
      inputStream: {
        name: "Live",
        type: "LiveStream",
        target: scannerElement,
        constraints: {
          facingMode: "environment",
          width: { ideal: 480 },
          height: { ideal: 360 }
        }
      },
      decoder: {
        readers: ["ean_reader"] // EAN-13 only (IMPORTANT)
      },
      locate: true
    },
    function (err) {
      if (err) {
        console.error(err);
        alert("Unable to access camera");
        return;
      }

      Quagga.start();
      scannerActive = true;

      // Attach listener only after start
      Quagga.onDetected(handleDetection);
    }
  );
}

/* ===============================
   HANDLE BARCODE DETECTION
================================ */
async function handleDetection(data) {
  if (scanLocked) return;

  const barcode = data.codeResult.code;

  // Reject invalid / noisy scans
  if (!isValidEAN13(barcode)) {
    console.warn("Rejected invalid barcode:", barcode);
    return;
  }

  scanLocked = true;
  Quagga.offDetected(handleDetection);

  console.log("Valid scanned barcode:", barcode);

  try {
    const product = await getProductByBarcode(barcode);

    if (!product) {
      alert("Product not found!");
      resumeScanner();
      return;
    }

    addToBill({
      barcode: barcode,
      name: product.name,
      price: product.price,
      weight: product.weight
    });

    sendToESP32({
  barcode: barcode,
  name: product.name,
  price: product.price,
  qty: 1
});

function sendToESP32(product) {
  // Only try ESP32 when running locally
  if (location.hostname !== "localhost") {
    console.log("ESP32 disabled on hosted site");
    return;
  }

  fetch("http://192.168.1.50/product", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(product)
  })
  .then(res => res.json())
  .then(data => console.log("ESP32:", data))
  .catch(err => console.error("ESP32 error:", err));
}



  } catch (error) {
    console.error("Scan processing error:", error);
  }

  resumeScanner();
}

/* ===============================
   RESUME SCANNING (COOLDOWN)
================================ */
function resumeScanner() {
  setTimeout(() => {
    scanLocked = false;
    Quagga.onDetected(handleDetection);
  }, 800); // 1.2s cooldown
}

/* ===============================
   STOP SCANNER (OPTIONAL)
================================ */
function stopScanner() {
  if (scannerActive) {
    Quagga.stop();
    Quagga.offDetected(handleDetection);
    scannerActive = false;
    scanLocked = false;
  }
}

/* ===============================
   CLEANUP ON PAGE EXIT
================================ */
window.addEventListener("beforeunload", () => {
  stopScanner();
});
