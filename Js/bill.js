/* ===============================
   BILL DATA (GLOBAL)
================================ */

window.billItems = [];
window.totalAmount = 0;
window.currentSessionId = null;   // 🔥 FIX

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
   REMOVE ITEMS
================================ */
window.removeOneFromBill = function (barcode) {
  const index = window.billItems.findIndex(i => i.barcode === barcode);
  if (index === -1) return;

  if (window.billItems[index].qty > 1) {
    window.billItems[index].qty--;
  } else {
    window.billItems.splice(index, 1);
  }

  window.renderBill();
};

window.removeItemCompletely = function (barcode) {
  window.billItems = window.billItems.filter(i => i.barcode !== barcode);
  window.renderBill();
};

/* ===============================
   FINALIZE CART (🔥 ONLY PLACE)
================================ */
window.finalizeCartAndProceed = async function () {

  if (!window.billItems.length) {
    alert("Cart is empty");
    return;
  }

  try {
    /* 1️⃣ CLEAR OLD DATA */
    await supabase.from("cart").delete().neq("id", 0);
    await supabase.from("cart_session").delete().neq("id", 0);
    await supabase.from("validation").delete().neq("id", 0);

    /* 2️⃣ CREATE SESSION */
    const { data: session, error: sessionErr } =
      await supabase
        .from("cart_session")
        .insert([{ status: "finalized" }])
        .select()
        .single();

    if (sessionErr) throw sessionErr;

    window.currentSessionId = session.id;   // 🔥 FIX
    localStorage.setItem("session_id", session.id);

    /* 3️⃣ INSERT CART */
    for (const item of window.billItems) {
      const { error } = await supabase.from("cart").insert([{
        session_id: window.currentSessionId,
        barcode: item.barcode,
        name: item.name,
        price: item.price,
        weight: item.weight,
        qty: item.qty
      }]);

      if (error) throw error;
    }

    /* 4️⃣ SAVE LOCALLY */
    localStorage.setItem("bill", JSON.stringify(window.billItems));
    localStorage.setItem("total", window.totalAmount);

    /* 5️⃣ GO TO PAYMENT */
    window.location.href = "pay.html";

  } catch (err) {
    console.error("Finalize error:", err);
    alert("Failed to proceed to payment. Please try again.");
  }
};
