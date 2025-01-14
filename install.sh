SERVICE_FILE="telemetry_proj.service"
EXECUTABLE="telemetry_sv"
INSTALL_DIR="/usr/bin"
SERVICE_DIR="/etc/systemd/system"

if [ "$(id -u)" -ne 0 ]; then
  echo "This script must be run as root." >&2
  exit 1
fi

echo "Installing executable to $INSTALL_DIR..."
cp "$EXECUTABLE" "$INSTALL_DIR/telemetry_sv"
chmod +x "$INSTALL_DIR/telemetry_sv"
echo "Executable installed."

echo "Installing service file to $SERVICE_DIR..."
cp "$SERVICE_FILE" "$SERVICE_DIR/telemetry_proj.service"
echo "Service file installed."

echo "Reloading systemd..."
systemctl daemon-reload

echo "Enabling the service to start on boot..."
systemctl enable telemetry_proj.service

echo "Starting the service..."
systemctl start telemetry_proj.service

systemctl status telemetry_proj.service

echo "Installation completed successfully."

