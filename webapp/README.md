# Smart Greenhouse Dashboard

A web app for monitoring and controlling greenhouse conditions.

## Prerequisites

- **Node.js** (with npm): [Install Node.js](https://nodejs.org/)
- **Python 3.8+**: [Install Python](https://www.python.org/)
- **Git**: [Install Git](https://git-scm.com/)

## Setup Instructions

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/smart-greenhouse-dashboard.git
cd smart-greenhouse-dashboard
```
### 2. Set Up the Frontend (Svelte)
```bash
cd frontend
npm install
npm run dev -- --open
```

### 3. Set Up the Backend (Flask)

```bash
cd backend
python -m venv venv
# Activate the virtual environment:
# On Windows:
.\venv\Scripts\activate
# On macOS/Linux:
source venv/bin/activate

pip install -r requirements.txt
python app.py
```