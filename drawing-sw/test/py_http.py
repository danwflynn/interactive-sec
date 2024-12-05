from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/api', methods=['POST'])
def handle_message():
	data = request.get_json()
	if not data:
		return jsonify({"error": "No JSON payload received"}), 400

	button = data.get("button")
	state = data.get("state")

	if button is None or state is None:
		return jsonify({"error": "Missing button or state in payload"}), 400

	print(f"Button {button} is {state}")
	return jsonify({"message": f"Received button {button} state {state}"}), 200

if __name__ == '__main__':
	app.run(host='0.0.0.0', port=5000)
