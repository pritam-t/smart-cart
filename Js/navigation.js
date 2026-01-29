/* ===============================
   NAVIGATION & SESSION CONTROL
================================ */

/* START SHOPPING SESSION */
function startSession() {
  localStorage.setItem("sessionStarted", "true");
}

/* HOME → SCAN PAGE */
function goToScanPage() {
  startSession();
  window.location.href = "scan.html";
}

function protectScanPage() {
  const cartId = localStorage.getItem("cart_id");

  if (!cartId) {
    console.warn("No cart selected, redirecting...");
    window.location.href = "select-cart.html";
  }
}


/* PAYMENT → HOME PAGE */
function goToHomePage() {
  localStorage.removeItem("bill");
  localStorage.removeItem("total");
  localStorage.removeItem("sessionStarted");
  window.location.href = "index.html";
}
