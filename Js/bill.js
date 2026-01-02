/* ===============================
   BILL DATA (GLOBAL)
================================ */

window.billItems = [];
window.totalAmount = 0;

/* ===============================
   ADD PRODUCT TO BILL
================================ */
window.addToBill = function (product) {
  const existingItem = window.billItems.find(
    item => item.barcode === product.barcode
  );

  if (existingItem) {
    existingItem.qty += 1;
  } else {
    window.billItems.push({
      barcode: product.barcode,
      name: product.name,
      price: product.price,
      weight: product.weight,
      qty: 1
    });
  }

  window.renderBill();
};

/* ===============================
   RENDER BILL TABLE
================================ */
window.renderBill = function () {
  const tbody = document.getElementById("billBody");
  const totalEl = document.getElementById("totalAmount");

  if (!tbody || !totalEl) return;

  tbody.innerHTML = "";
  window.totalAmount = 0;

  window.billItems.forEach((item, index) => {
    const rowTotal = item.qty * item.price;
    window.totalAmount += rowTotal;

    const row = document.createElement("tr");
    row.innerHTML = `
  <td>${index + 1}</td>
  <td>${item.name}</td>
  <td>${item.qty}</td>
  <td>₹${rowTotal}</td>
  <td>${item.weight}</td>
  <td>
    <button onclick="removeOneFromBill('${item.barcode}')">−</button>
    <button onclick="removeItemCompletely('${item.barcode}')">🗑</button>
  </td>
`;
    tbody.appendChild(row);
  });

  totalEl.innerText = "₹" + window.totalAmount;
};

/* ===============================
   SAVE BILL & MOVE TO PAYMENT
================================ */
window.saveBillAndProceed = function () {
  localStorage.setItem("bill", JSON.stringify(window.billItems));
  localStorage.setItem("total", window.totalAmount);
  window.location.href = "pay.html";
};

/* ===============================
   CLEAR BILL (OPTIONAL)
================================ */
window.clearBill = function () {
  window.billItems = [];
  window.totalAmount = 0;
  window.renderBill();
};

window.removeOneFromBill = function (barcode) {
  const index = window.billItems.findIndex(
    item => item.barcode === barcode
  );

  if (index === -1) return;

  if (window.billItems[index].qty > 1) {
    window.billItems[index].qty -= 1;
  } else {
    window.billItems.splice(index, 1);
  }

  window.renderBill();
};

window.removeItemCompletely = function (barcode) {
  window.billItems = window.billItems.filter(
    item => item.barcode !== barcode
  );

  window.renderBill();
};
