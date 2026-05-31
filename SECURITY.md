# Security Policy

## Supported Versions

This is a hobby robotics research project. Only the latest commit on `main` is actively maintained.

| Version | Supported |
|---|---|
| Latest (`main`) | ✅ Yes |
| Older commits | ❌ No |

---

## Reporting a Vulnerability

If you discover a security issue — particularly anything that could allow remote code execution, credential exposure, or unauthorised motor control — please report it responsibly rather than opening a public issue.

**Contact:** Open a private security advisory via GitHub → Security → Advisories → New draft security advisory.

Please include:
- A description of the vulnerability
- The affected firmware node (main-controller / ai-camera / cyd-display)
- Steps to reproduce
- Potential impact

I will respond within 7 days and coordinate a fix before public disclosure.

---

## Known Security Considerations

**Credentials in `Secrets.h`:** Wi-Fi passwords, OTA passwords, and ThingSpeak API keys are stored in `Secrets.h`, which is excluded from version control via `.gitignore`. Never commit this file.

**OTA firmware:** OTA is password-protected per node but transmitted over unencrypted Wi-Fi. For production deployments, restrict OTA access to a trusted local network.

**ThingSpeak write key:** If your write API key is exposed, regenerate it immediately from your ThingSpeak channel API Keys tab.

**Motor failsafe:** The BTS7960 motor controller includes a 500 ms command timeout failsafe. If Bluetooth or the control task fails, motors stop automatically.

**HTTP camera stream:** The OV2640 MJPEG stream and AI detection endpoints are unauthenticated HTTP. Do not expose them to the public internet.