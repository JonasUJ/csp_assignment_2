import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

def load_data(filename):
    return pd.read_csv(filename)

def convert_time_to_throughput(df):
    # time is in nanoseconds per million queries
    # throughput = 1e9 / time => million queries per second
    df['Throughput (M q/s)'] = 1e9 / df['time']
    return df

def plot_per_structure(df, output_dir):
    os.makedirs(output_dir, exist_ok=True)

    for structure, group in df.groupby('structure'):
        plt.figure(figsize=(10, 6))

        # Plot each thread level as a separate line
        for thread_level, thread_group in group.groupby('thread_level'):
            sorted_group = thread_group.sort_values('size')  # sort to get a proper line plot
            plt.plot(sorted_group['size'], sorted_group['Throughput (M q/s)'],
                     marker='o', label=f'Threads: {thread_level}')

        plt.xlabel("Size")
        plt.ylabel("Throughput (Million Queries per Second)")
        plt.title(f"Throughput vs Size — {structure}")
        plt.legend(title="Thread Level")
        plt.grid(True)

        # Clean filename for saving
        filename = f"{structure.replace(' ', '_').replace('+', 'plus')}.png"
        plt.savefig(os.path.join(output_dir, filename))
        plt.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot_throughput_per_structure.py <data.csv> [output_dir]")
        sys.exit(1)

    filename = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "plots"

    df = load_data(filename)
    df = convert_time_to_throughput(df)
    plot_per_structure(df, output_dir)
