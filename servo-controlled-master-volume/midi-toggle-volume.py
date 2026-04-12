import rtmidi
import time

def list_midi_ports():
    """Lists all available MIDI output ports.
    (This function is now primarily for debugging or initial setup if needed,
    as the port number can be hardcoded directly in __main__.)
    """
    midiout = rtmidi.MidiOut()
    ports = midiout.get_ports()
    print("Available MIDI Output Ports:")
    if not ports:
        print("  No MIDI output ports found.")
        print("  Please ensure your Edirol UM-1 is connected and drivers are installed.")
        return None
    for i, port in enumerate(ports):
        print(f"  [{i}] {port}")
    print("\nPlease note the number corresponding to your Edirol UM-1 (e.g., 'EDIROL UM-1', 'UM-1', etc.).")
    return ports

def send_cc_toggle(port_number, channel, cc_number, value1, value2, delay_seconds):
    """
    Toggles a MIDI CC message between two values in a continuous loop.

    Args:
        port_number (int): The index of the MIDI output port to use.
        channel (int): The MIDI channel (1-16).
        cc_number (int): The Control Change number (0-127).
        value1 (int): The first CC value (0-127).
        value2 (int): The second CC value (0-127).
        delay_seconds (float): The delay in seconds between sending each value.
    """
    midiout = rtmidi.MidiOut()
    ports = midiout.get_ports()

    # Direct check for the specific port based on port_number
    if not ports or port_number >= len(ports) or port_number < 0:
        print(f"Error: Invalid MIDI port number {port_number}. Please check available ports.")
        return

    port_name = ports[port_number]
    print(f"\nAttempting to open MIDI port: {port_name}")
    try:
        midiout.open_port(port_number)
    except rtmidi.midiutil.PortNotOpenError as e:
        print(f"Error opening port {port_number}: {e}")
        print("Is the port already in use or does the user lack permissions?")
        return

    print(f"Toggling MIDI CC message to '{port_name}' on Channel {channel} (CC #{cc_number})")
    print(f"Values will toggle between {value1} and {value2} every {delay_seconds} seconds. Press Ctrl+C to stop.")

    # Adjust channel to be 0-indexed for MIDI messages
    midi_channel = channel - 1

    try:
        while True: # Loop indefinitely for continuous toggling
            # Send the first value
            clamped_value1 = max(0, min(127, value1))
            midi_message1 = [0xB0 + midi_channel, cc_number, clamped_value1]
            midiout.send_message(midi_message1)
            print(f"Sent CC {cc_number} value {clamped_value1} on Channel {channel}")
            time.sleep(delay_seconds)

            # Send the second value
            clamped_value2 = max(0, min(127, value2))
            midi_message2 = [0xB0 + midi_channel, cc_number, clamped_value2]
            midiout.send_message(midi_message2)
            print(f"Sent CC {cc_number} value {clamped_value2} on Channel {channel}")
            time.sleep(delay_seconds)

    except KeyboardInterrupt:
        print("\nToggle interrupted by user.")
    finally:
        print("\nToggle finished. Closing MIDI port.")
        midiout.close_port()


if __name__ == "__main__":
    print("--- MIDI Interface Identification ---")
    # No need to list all ports if the number is known, but keeping function for reference
    # available_ports = list_midi_ports()

    # --- Configuration for MIDI CC Toggle ---
    # The port number is now hardcoded as requested.
    UM1_PORT_NUMBER = 1 # <--- HARDCODED TO 20 AS REQUESTED

    MIDI_CHANNEL = 1     # MIDI channel (1-16)
    CC_NUMBER = 7        # CC #7 is typically Volume
    VALUE_ON = 127       # The "on" value for the toggle
    VALUE_OFF = 0        # The "off" value for the toggle
    TOGGLE_DELAY = 5     # seconds (delay between sending each value)

    # Call send_cc_toggle directly with the hardcoded port number
    # The initial port listing is no longer strictly necessary, but `send_cc_toggle`
    # still performs a check to ensure the port exists before trying to open it.
    send_cc_toggle(UM1_PORT_NUMBER, MIDI_CHANNEL, CC_NUMBER, VALUE_OFF, VALUE_ON, TOGGLE_DELAY)
