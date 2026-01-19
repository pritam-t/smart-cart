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
   REMOVE ONE QUANTITY
================================ */
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

/* ===============================
   REMOVE ITEM COMPLETELY
================================ */
window.removeItemCompletely = function (barcode) {
  window.billItems = window.billItems.filter(
    item => item.barcode !== barcode
  );

  window.renderBill();
};

/* ===============================
   CLEAR BILL (OPTIONAL)
================================ */
window.clearBill = function () {
  window.billItems = [];
  window.totalAmount = 0;
  window.renderBill();
};

/* ===============================
   FINALIZE CART & PROCEED TO PAY
   ✅ ONLY PLACE SUPABASE IS UPDATED
================================ */
window.finalizeCartAndProceed = async function () {

  if (!window.billItems.length) {
    alert("Cart is empty");
    return;
  }

  try {
    /* 1️⃣ CREATE UNIQUE SESSION ID */
    const sessionId = crypto.randomUUID();

    /* 2️⃣ PUSH CART ITEMS */
    for (const item of window.billItems) {
      const { error } = await supabase.from("cart").insert([{
        session_id: sessionId,
        barcode: item.barcode,
        name: item.name,
        price: item.price,
        weight: item.weight,
        qty: item.qty
      }]);

      if (error) throw error;
    }

    /* 3️⃣ FINALIZE SESSION (ESP32 TRIGGER) */
    const { error: sessionError } = await supabase
      .from("cart_session")
      .insert([{
        session_id: sessionId,
        status: "finalized"
      }]);

    if (sessionError) throw sessionError;

    /* 4️⃣ SAVE LOCALLY FOR PAYMENT PAGE */
    localStorage.setItem("bill", JSON.stringify(window.billItems));
    localStorage.setItem("total", window.totalAmount);
    localStorage.setItem("session_id", sessionId);

    /* 5️⃣ GO TO PAYMENT */
    window.location.href = "pay.html";

  } catch (err) {
    console.error("Finalize error:", err);
    alert("Failed to proceed to payment. Please try again.");
  }
};
