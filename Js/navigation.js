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

/* PROTECT SCAN PAGE */
function protectScanPage() {
  if (!localStorage.getItem("sessionStarted")) {
    window.location.href = "index.html";
  }
}

/* PAYMENT → HOME PAGE */
function goToHomePage() {
  localStorage.removeItem("bill");
  localStorage.removeItem("total");
  localStorage.removeItem("sessionStarted");
  window.location.href = "index.html";
}
