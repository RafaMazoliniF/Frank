from flask import Flask, render_template, request, jsonify, redirect, url_for
import sqlite3, os

app = Flask(__name__)
app.secret_key = "compiler"

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(BASE_DIR, 'database.db') 

def init_db():
    con = sqlite3.connect(DB_PATH)
    c = con.cursor()

    c.execute("CREATE TABLE IF NOT EXISTS files (id INTEGER PRIMARY KEY AUTOINCREMENT, file BLOB NOT NULL)")
    
    con.commit()
    con.close()

@app.route("/")
def direct():
     return render_template('home.html')

if __name__ == "__main__":        
    init_db()
    app.run(host="0.0.0.0", port=5000, debug=True, use_reloader=True)
