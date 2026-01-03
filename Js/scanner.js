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
   SEND DATA TO ESP32 (LOCAL ONLY)
================================ */
function sendToESP32(product) {
  // ESP32 only available in local environment
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
    .then(data => console.log("ESP32 response:", data))
    .catch(err => console.error("ESP32 error:", err));
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
        readers: ["ean_reader"] // EAN-13 only
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
      Quagga.onDetected(handleDetection);
    }
  );
}

/* ===============================
   HANDLE BARCODE DETECTION
================================ */
async function handleDetection(data) {
  const barcode = data.codeResult.code;

  // Stop rapid duplicate scans
  Quagga.offDetected(handleDetection);

  const product = await getProductByBarcode(barcode);

  if (!product) {
    alert("Product not found!");
    resumeScanner();
    return;
  }

  // 1️⃣ Add to local bill (UI)
  addToBill({
    barcode: barcode,
    name: product.name,
    price: product.price,
    weight: product.weight
  });

  // 2️⃣ Save same data to Supabase (CLOUD)
  sendToSupabase({
    barcode: barcode,
    name: product.name,
    price: product.price
  });

  console.log("Scanned & saved:", barcode);

  resumeScanner();
}


/* ===============================
   RESUME SCANNING (COOLDOWN)
================================ */
function resumeScanner() {
  setTimeout(() => {
    scanLocked = false;
    Quagga.onDetected(handleDetection);
  }, 800);
}

/* ===============================
   STOP SCANNER
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
window.addEventListener("beforeunload", stopScanner);

async function sendToSupabase(product) {
  const { error } = await window.supabase
    .from("cart")
    .insert([
      {
        barcode: product.barcode,
        name: product.name,
        price: product.price,
        qty: 1,
        source: "web"
      }
    ]);

  if (error) {
    console.error("Supabase insert error:", error);
  } else {
    console.log("Saved to Supabase:", product.barcode);
  }
}
