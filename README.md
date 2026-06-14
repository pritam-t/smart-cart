# SmartCart

## AI-Assisted Smart Digital Billing and Product Awareness System

SmartCart is an intelligent retail automation platform that combines IoT, cloud computing, and artificial intelligence to transform the shopping experience. The system eliminates traditional checkout queues, validates purchases through real-time weight verification, and provides customers with personalized product insights and health recommendations.

Unlike conventional billing systems, SmartCart not only automates checkout but also helps users make safer and more informed purchasing decisions.

---

## Problem Statement

Traditional retail shopping faces several challenges:

* Long checkout queues and waiting times
* Manual billing errors
* Product mismatch and shoplifting risks
* Lack of consumer awareness regarding ingredients and health impacts
* High operational costs due to cashier dependency
* No personalized shopping assistance

Customers often purchase products without understanding their contents, nutritional value, allergens, sugar levels, or potential health concerns.

---

## Our Solution

SmartCart provides a fully integrated smart shopping ecosystem consisting of:

* Mobile-based barcode scanning
* Real-time digital billing
* IoT-based cart weight verification
* Cloud synchronization
* AI-powered product analysis
* Personalized shopping recommendations

The platform ensures that every item added to the bill matches the physical contents of the cart before payment is authorized.

---

## Key Features

### Smart Digital Billing

Customers scan products directly using their device.

* Instant bill generation
* Real-time cart updates
* Queue-free shopping experience

---

### Weight Verification System

Each product contains predefined weight information.

The system compares:

Expected Cart Weight
vs
Actual Cart Weight

using:

* Load Cell
* HX711 Amplifier
* ESP8266/ESP32

If a mismatch is detected:

* Payment is blocked
* Customer receives a warning
* Store staff can review the discrepancy

This helps prevent:

* Unbilled products
* Accidental mistakes
* Shoplifting

---

### AI Product Awareness Assistant

After scanning a product, the AI assistant explains:

* Ingredients
* Nutritional information
* Sugar content
* Artificial additives
* Safety warnings
* Storage recommendations

Example:

"This product contains high sugar content and is not recommended for daily consumption."

"Contains caffeine. Consumption by children should be limited."

"Store in a cool and dry place away from direct sunlight."

The objective is to convert complex product labels into simple consumer-friendly explanations.

---

### Personalized AI Recommendations

SmartCart goes beyond product explanation by offering personalized suggestions.

Based on:

* Previously scanned products
* Shopping behavior
* Product categories
* Dietary preferences

The assistant can provide recommendations such as:

* Healthier alternatives
* Low sugar substitutes
* High protein options
* Budget-friendly choices
* Similar products with better nutritional value

Example:

"Since you frequently purchase high-sugar beverages, you may consider this low-sugar alternative."

"This product contains fewer preservatives compared to similar items."

---

### Multi-Cart Management

Supports multiple smart carts simultaneously.

Each cart operates independently using:

* Cart ID
* Session ID
* Cloud synchronization

This allows deployment across large retail environments.

---

### Digital Invoice Generation

After successful verification:

* Payment is completed
* Digital receipt is generated
* PDF invoice can be downloaded

---

## System Architecture

Customer Device
↓
Barcode Scanner
↓
Web Application
↓
Supabase Cloud Backend
↓
Cart Session Management
↓
ESP8266 + HX711 + Load Cell
↓
Weight Verification
↓
AI Product Analysis Engine
↓
Payment Authorization

---

## Technology Stack

### Frontend

* HTML
* CSS
* JavaScript

### Backend

* Supabase
* REST APIs

### Hardware

* ESP8266 / ESP32
* HX711 Amplifier
* Load Cell Sensor

### Database

* Supabase PostgreSQL

### AI Layer

* Large Language Models
* Product Ingredient Analysis
* Personalized Recommendation Engine

---

## Project Workflow

### Step 1

Customer selects a SmartCart.

### Step 2

Products are scanned using the web application.

### Step 3

Products are added to the live bill.

### Step 4

AI assistant analyzes the product.

### Step 5

Health insights and recommendations are displayed.

### Step 6

Customer proceeds to payment.

### Step 7

ESP8266 verifies cart weight.

### Step 8

System compares expected and actual weight.

### Step 9

If verification succeeds:

* Payment is enabled

Otherwise:

* Payment remains blocked

### Step 10

Invoice is generated.

---

## Advantages

### For Customers

* Faster shopping
* No checkout queues
* Better product awareness
* Personalized recommendations

### For Retailers

* Reduced manpower costs
* Improved fraud detection
* Enhanced customer experience

### For Society

* Health-conscious shopping
* Improved consumer awareness
* Sustainable paperless billing

---

## Future Enhancements

* Voice-enabled shopping assistant
* Computer Vision product recognition
* AI diet planning integration
* Customer profile personalization
* Smart inventory forecasting
* UPI and wallet integration
* Mobile application deployment

---

## Estimated Production Cost

| Component                | Approximate Cost |
| ------------------------ | ---------------- |
| ESP8266 / ESP32          | ₹250 - ₹400      |
| HX711 Module             | ₹120 - ₹150      |
| Load Cell                | ₹300 - ₹500      |
| Power System             | ₹300 - ₹400      |
| Mounting and Casing      | ₹400 - ₹600      |
| Miscellaneous Components | ₹300 - ₹500      |

Estimated Total Cost per SmartCart:

₹2500 - ₹3500

This is significantly lower than RFID-based smart cart solutions.

---

## Vision

To create an intelligent, affordable, and scalable retail ecosystem that combines automated billing, fraud prevention, consumer education, and personalized AI assistance into a single shopping experience.

---

## License

This project is intended for educational, research, and innovation purposes. Commercial deployment may require additional compliance, security, and infrastructure considerations.
