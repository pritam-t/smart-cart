/* ===============================
   JSON DATA LOADER
================================ */

/**
 * Loads product data from products.json
 * @returns {Promise<Object>} product data indexed by barcode
 */
async function loadProducts() {
    try {
        const response = await fetch("data/products.json");

        if (!response.ok) {
            throw new Error("Failed to load products.json");
        }

        const products = await response.json();
        return products;
    } catch (error) {
        console.error("Error loading product data:", error);
        return {};
    }
}

/**
 * Fetch a single product using barcode
 * @param {string} barcode
 * @returns {Promise<Object|null>}
 */
async function getProductByBarcode(barcode) {
    const products = await loadProducts();
    return products[barcode] || null;
}
