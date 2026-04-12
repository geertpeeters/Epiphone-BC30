import rtmidi
import time

def list_midi_ports():
    """Lists all available MIDI output ports."""
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

def send_cc_sweep(port_number, channel, cc_number, start_value, end_value, duration_seconds):
    """
    Sends MIDI CC messages, sweeping a value gently from start to end, and then back from end to start,
    in a continuous loop over a specified duration for each direction.

    Args:
        port_number (int): The index of the MIDI output port to use.
        channel (int): The MIDI channel (1-16).
        cc_number (int): The Control Change number (0-127).
        start_value (int): The starting CC value (0-127).
        end_value (int): The ending CC value (0-127).
        duration_seconds (float): The total duration of one sweep direction (e.g., 0 to 127) in seconds.
    """
    midiout = rtmidi.MidiOut()
    ports = midiout.get_ports()

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

    print(f"Sending MIDI CC messages to '{port_name}' on Channel {channel} (CC #{cc_number})...")
    print("Sweeping from 0 to 127 and back in a continuous loop. Press Ctrl+C to stop.")

    # Adjust channel to be 0-indexed for MIDI messages
    midi_channel = channel - 1

    # Calculate steps and delay for one direction
    num_steps = abs(end_value - start_value) + 1
    if num_steps <= 1: # Handle cases where there's no actual sweep (e.g., 0 to 0)
        print("Start and end values are too close or same; no sweep needed.")
        midiout.close_port()
        return

    sleep_time = duration_seconds / num_steps

    try:
        while True: # Loop indefinitely for continuous sweep
            # Sweep from start_value to end_value
            current_value = start_value
            step_direction = 1 if end_value > start_value else -1

            for _ in range(num_steps):
                if not (0 <= current_value <= 127):
                    print(f"Warning: CC value {current_value} out of range (0-127). Clamping.")
                    current_value = max(0, min(127, current_value))

                midi_message = [0xB0 + midi_channel, cc_number, current_value]
                midiout.send_message(midi_message)
                # print(f"Sent CC {cc_number} value {current_value} on Channel {channel}") # For debugging

                current_value += step_direction
                time.sleep(sleep_time)

            # Ensure the end value is hit precisely
            midiout.send_message([0xB0 + midi_channel, cc_number, end_value])
            time.sleep(sleep_time) # Small pause at the end of the first sweep

            # Sweep from end_value back to start_value
            current_value = end_value
            step_direction = -1 if end_value > start_value else 1

            for _ in range(num_steps):
                if not (0 <= current_value <= 127):
                    print(f"Warning: CC value {current_value} out of range (0-127). Clamping.")
                    current_value = max(0, min(127, current_value))

                midi_message = [0xB0 + midi_channel, cc_number, current_value]
                midiout.send_message(midi_message)
                # print(f"Sent CC {cc_number} value {current_value} on Channel {channel}") # For debugging

                current_value += step_direction
                time.sleep(sleep_time)

            # Ensure the start value is hit precisely
            midiout.send_message([0xB0 + midi_channel, cc_number, start_value])
            time.sleep(sleep_time) # Small pause at the end of the second sweep

    except KeyboardInterrupt:
        print("\nSweep interrupted by user.")
    finally:
        print("\nSweep finished. Closing MIDI port.")
        midiout.close_port()


if __name__ == "__main__":
    print("--- MIDI Interface Identification ---")
    available_ports = list_midi_ports()

    # --- Configuration for MIDI CC Sweep ---
    # IMPORTANT: Replace 0 with the actual port number of your Edirol UM-1
    # You will find this number from the "Available MIDI Output Ports" list above.
    UM1_PORT_NUMBER = 1 # <--- CHANGE THIS TO YOUR EDIROL UM-1'S PORT NUMBER

    MIDI_CHANNEL = 1    # MIDI channel (1-16)
    CC_NUMBER = 7       # CC #7 is typically Volume
    START_VALUE = 0     # Sweep from 0
    END_VALUE = 127     # Sweep to 127
    SWEEP_DURATION = 3  # seconds (for ONE direction, e.g., 0 to 127)

    if available_ports and 0 <= UM1_PORT_NUMBER < len(available_ports):
        # Uncomment the line below to run the CC sweep after identifying your port!
        send_cc_sweep(UM1_PORT_NUMBER, MIDI_CHANNEL, CC_NUMBER, START_VALUE, END_VALUE, SWEEP_DURATION)
    else:
        print("\nCannot perform sweep: No valid port selected or available ports not found.")
        print("Please ensure your Edirol UM-1 is connected and recognized by your system.")
