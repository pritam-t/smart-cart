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
   (ONLY PLACE WHERE SUPABASE IS UPDATED)
================================ */
window.finalizeCartAndProceed = async function () {

  const bill = window.billItems;

  if (!bill || bill.length === 0) {
    alert("Cart is empty");
    return;
  }

  try {
    /* 🧹 CLEAR OLD DATA (IMPORTANT) */
    await supabase.from("cart").delete().neq("id", 0);
    await supabase.from("cart_session").delete().neq("id", 0);

    /* 1️⃣ PUSH CART TO SUPABASE */
    for (const item of bill) {
      const { error } = await supabase.from("cart").insert([{
        barcode: item.barcode,
        name: item.name,
        price: item.price,
        weight: item.weight,
        qty: item.qty,
        source: "web"
      }]);

      if (error) throw error;
    }

    /* 2️⃣ CREATE FINALIZED SESSION (ESP32 SIGNAL) */
    const { error: sessionError } =
      await supabase.from("cart_session").insert([{
        status: "finalized"
      }]);

    if (sessionError) throw sessionError;

    /* 3️⃣ SAVE LOCALLY FOR PAYMENT PAGE */
    localStorage.setItem("bill", JSON.stringify(bill));
    localStorage.setItem("total", window.totalAmount);

    /* 4️⃣ NAVIGATE TO PAYMENT */
    window.location.href = "pay.html";

  } catch (err) {
    console.error("Finalize error:", err);
    alert("Failed to proceed to payment. Please try again.");
  }
};
