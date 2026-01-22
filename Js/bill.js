/* ===============================
   GLOBAL BILL STATE
================================ */

window.billItems = [];
window.totalAmount = 0;

/* ===============================
   GET SELECTED CART ID
================================ */
const CART_ID = Number(localStorage.getItem("cart_id"));

if (!CART_ID) {
  alert("No cart selected. Please select a cart first.");
  throw new Error("cart_id missing");
}

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

  renderBill();
};

/* ===============================
   RENDER BILL
================================ */
function renderBill() {
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
}

window.renderBill = renderBill;

/* ===============================
   REMOVE ONE ITEM
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

  renderBill();
};

/* ===============================
   REMOVE ITEM COMPLETELY
================================ */
window.removeItemCompletely = function (barcode) {
  window.billItems = window.billItems.filter(
    item => item.barcode !== barcode
  );
  renderBill();
};

/* ===============================
   CLEAR BILL
================================ */
window.clearBill = function () {
  window.billItems = [];
  window.totalAmount = 0;
  renderBill();
};

/* ===============================
   FINALIZE CART & PROCEED
   (ONLY SUPABASE WRITE POINT)
================================ */
window.finalizeCartAndProceed = async function () {

  if (window.billItems.length === 0) {
    alert("Cart is empty");
    return;
  }

  try {
    console.log("🧹 Clearing old data for cart:", CART_ID);

    /* 1️⃣ CLEAR OLD DATA FOR THIS CART ONLY */
    await supabase.from("cart").delete().eq("cart_id", CART_ID);
    await supabase.from("cart_session").delete().eq("cart_id", CART_ID);
    await supabase.from("validation").delete().eq("cart_id", CART_ID);

    /* 2️⃣ CREATE NEW SESSION (ESP SIGNAL) */
    const { data: sessionData, error: sessionError } = await supabase
      .from("cart_session")
      .insert({
        cart_id: CART_ID,
        status: "finalized"
      })
      .select("id")
      .single();

    if (sessionError) throw sessionError;

    const SESSION_ID = sessionData.id;
    console.log("🆔 Session created:", SESSION_ID);

    /* 3️⃣ INSERT CART ITEMS */
    for (const item of window.billItems) {
      const { error } = await supabase.from("cart").insert({
        cart_id: CART_ID,
        session_id: SESSION_ID,
        barcode: item.barcode,
        name: item.name,
        price: item.price,
        weight: item.weight,
        qty: item.qty
      });

      if (error) throw error;
    }

    /* 4️⃣ SAVE LOCALLY FOR PAY PAGE */
    localStorage.setItem("bill", JSON.stringify(window.billItems));
    localStorage.setItem("total", window.totalAmount);
    localStorage.setItem("session_id", SESSION_ID);

    /* 5️⃣ GO TO PAYMENT PAGE */
    window.location.href = "pay.html";

  } catch (err) {
    console.error("❌ Finalize error:", err);
    alert("Failed to proceed to payment. Please try again.");
  }
};
