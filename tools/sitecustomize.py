# Bypass WMI queries in platform.py to prevent hanging when Windows WMI service is slow/unresponsive
import platform

def _wmi_bypass(*args, **kwargs):
    raise OSError("WMI bypassed for performance")

try:
    platform._wmi_query = _wmi_bypass
except Exception:
    pass
