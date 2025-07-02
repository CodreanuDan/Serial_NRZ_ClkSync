import matplotlib.pyplot as plt
import numpy as np
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))  
DATA_DIR = os.path.join(BASE_DIR, '..', 'data')

tx_file = os.path.join(DATA_DIR, 'Transmitter_Sequence.txt')
rx_file = os.path.join(DATA_DIR, 'Receiver_Sequence.txt')

def parse_file(filename):
    times = []
    data_bits = []
    clock_bits = []

    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or '->' not in line:
                continue
            try:
                time_part, bits_part = line.split('->')
                time_str = time_part.strip()
                data_str, clock_str = bits_part.strip().split()
                times.append(time_str)
                data_bits.append(int(data_str))
                clock_bits.append(int(clock_str))
            except:
                continue

    def time_to_seconds(tstr):
        h, m, s = tstr.split(':')
        s = float(s)
        return int(h)*3600 + int(m)*60 + s

    times_sec = [time_to_seconds(t) for t in times]

    return np.array(times_sec), np.array(data_bits), np.array(clock_bits)

def bits_to_ascii_string(bits):
    chars = []
    for i in range(0, len(bits), 8):
        byte_bits = bits[i:i+8]
        if len(byte_bits) < 8:
            break
        val = 0
        for bit_idx, bit in enumerate(byte_bits):
            val |= (bit << (7 - bit_idx))
        # Ignore non-printable characters for safety
        if 32 <= val <= 126:
            chars.append(chr(val))
        else:
            chars.append('?')
    return ''.join(chars)

def add_decoded_text(ax, bits, ypos=1.15, label=''):
    decoded = bits_to_ascii_string(bits)
    # Aranjăm textul pe o linie
    ax.text(0, ypos, f"{label}: {decoded}", fontsize=10, color='blue', ha='left', va='bottom', family='monospace')

def main():
    tx_t, tx_data, tx_clk = parse_file(tx_file)
    rx_t, rx_data, rx_clk = parse_file(rx_file)

    fig, axs = plt.subplots(4,1, figsize=(18,10), sharex=True)

    axs[0].step(tx_t, tx_data, where='post', label='TX Data', linewidth=1.5)
    axs[0].set_ylim(-0.2,1.2)
    axs[0].set_yticks([0,1])
    axs[0].set_ylabel('TX Data')
    add_decoded_text(axs[0], tx_data, label='TX ASCII')

    axs[1].step(tx_t, tx_clk, where='post', label='TX Clock', linewidth=1.5, color='orange')
    axs[1].set_ylim(-0.2,1.2)
    axs[1].set_yticks([0,1])
    axs[1].set_ylabel('TX Clock')

    axs[2].step(rx_t, rx_data, where='post', label='RX Data', linewidth=1.5, color='green')
    axs[2].set_ylim(-0.2,1.2)
    axs[2].set_yticks([0,1])
    axs[2].set_ylabel('RX Data')
    add_decoded_text(axs[2], rx_data, label='RX ASCII')

    axs[3].step(rx_t, rx_clk, where='post', label='RX Clock', linewidth=1.5, color='red')
    axs[3].set_ylim(-0.2,1.2)
    axs[3].set_yticks([0,1])
    axs[3].set_ylabel('RX Clock')
    axs[3].set_xlabel('Time (seconds)')

    for ax in axs:
        ax.grid(True)

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
