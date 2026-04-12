import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
import matplotlib.patches as mpatches


"""PARTITION VISUALIZER - Frame by Frame
Author: Huzaifa Ahmad Awan
Course: Just beyond P · Max-Planck-Institut für Informatik · 2025/26

Description:
This script provides a frame-by-frame visualization of the partitioning process based
on a user-defined distance metric and a specified permutation of points. It allows 
users to see how clusters are formed step by step, with interactive navigation to move
through the frames. The visualization includes the remaining points (R), the current 
cluster (Q), and the center point with a β-radius circle, along with a side panel 
showing completed clusters and the permutation used.

Usage:
1. Run the script and input the metric points as R^2 coordinates in the format:
Example Input:
metric points as R^2 coordinates: (0,0); (2,0); (4,2); (2,2); (1,3); (10,4); (12,4); (11,5); (10,7); (12,7); (15,15); (16,15); (15,16); (16,16); (5,16); (6,16); (5,15); (6,15); (7,17); (8,18); (9,19); (9,20)
permutation: 3 1 7 0 9 8 12 2 4 14 15 5 10 21 20 6 11 13 16 19 18 17
β (radius parameter): 5
"""

def parse_metric_input(input_string):
    """
    Parse input like: (0,1); (8,9); (5,3)
    Returns: numpy array of coordinates and distance metric
    """
    # Remove spaces and split by semicolon
    points_str = input_string.strip().replace(" ", "").split(";")
    
    coords = []
    for point_str in points_str:
        # Remove parentheses and split by comma
        point_str = point_str.strip().strip("()")
        if point_str:
            x, y = map(float, point_str.split(","))
            coords.append([x, y])
    
    coords = np.array(coords)
    n = len(coords)
    
    # Compute distance metric
    metric = np.zeros((n, n))
    for i in range(n):
        for j in range(n):
            metric[i, j] = np.linalg.norm(coords[i] - coords[j])
    
    return coords, metric


def visualize_partition_frames(coords, metric, permutation, beta):
    """
    Frame-by-frame visualization of partition process with interactive navigation.
    Shows R (remaining points), Q (current cluster), and center with β-radius.
    Also displays already-formed clusters on the side.
    """
    n = len(coords)
    R = set(range(n))  # All points initially
    clusters = []
    frames = []
    permutation_str = " → ".join(map(str, permutation))
    
    # Generate frames showing each step
    for step, idx in enumerate(permutation):
        if idx not in R:
            continue
        
        # Find Qi: points within distance beta from center point idx
        Qi = [p for p in R if metric[idx, p] <= beta]
        
        # Store frame state with clusters formed so far
        frames.append({
            'step': step,
            'center': idx,
            'cluster': Qi,
            'remaining': R.copy(),
            'completed_clusters': [c.copy() for c in clusters]
        })
        
        # Add to clusters and remove from R
        if Qi:
            clusters.append(Qi)
            for p in Qi:
                R.remove(p)
    
    # Add final frame(s) for remaining unpartitioned points
    remaining_points = list(R)
    for remaining_point in remaining_points:
        clusters.append([remaining_point])
    
    # Add final frame showing all clusters completed
    frames.append({
        'step': len(permutation),
        'center': None,
        'cluster': [],
        'remaining': set(),
        'completed_clusters': [c.copy() for c in clusters]
    })
    
    # Interactive visualization with navigation
    num_frames = len(frames)
    print(f"Total frames to display: {num_frames}")
    
    class FrameNavigator:
        def __init__(self, frames, coords, metric, beta, permutation_str):
            self.frames = frames
            self.coords = coords
            self.metric = metric
            self.beta = beta
            self.permutation_str = permutation_str
            self.current_frame = 0
            self.num_frames = len(frames)
            self.fig = None
            self.ax = None
            self.ax_text = None
            self.buttons = {}
            self.first_draw = True
            
        def draw_frame(self, frame_idx):
            """Draw a specific frame"""
            frame = self.frames[frame_idx]
            
            is_first = self.first_draw
            
            if is_first or self.fig is None:
                # Create figure with main plot and text area for clusters only on first draw
                self.fig, (self.ax, self.ax_text) = plt.subplots(1, 2, figsize=(16, 9), gridspec_kw={'width_ratios': [0.6, 0.4]})
                # Adjust layout to make room for buttons
                self.fig.subplots_adjust(left=0.07, right=0.96, top=0.93, bottom=0.15, wspace=0.3)
                self.fig.canvas.mpl_connect('key_press_event', self._handle_key)
                self.first_draw = False
            else:
                # Clear previous content
                self.ax.clear()
                self.ax_text.clear()
            
            ax = self.ax
            ax_text = self.ax_text
            
            # Get all points that have been used (remaining + completed)
            all_points = set(range(len(self.coords)))
            used_points = set()
            for cluster in frame['completed_clusters']:
                used_points.update(cluster)
            used_points.update(frame['remaining'])
            if frame['center'] is not None:
                used_points.add(frame['center'])
            used_points.update(frame['cluster'])
            
            # Plot completed cluster points (faded)
            for cluster_idx, cluster in enumerate(frame['completed_clusters']):
                cluster_coords = self.coords[cluster]
                ax.scatter(cluster_coords[:, 0], cluster_coords[:, 1], 
                          s=150, alpha=0.3, edgecolors='gray', linewidth=1, 
                          zorder=1)
            
            # Plot remaining points (R) in gray
            if frame['remaining']:
                remaining_coords = self.coords[list(frame['remaining'])]
                ax.scatter(remaining_coords[:, 0], remaining_coords[:, 1], 
                          s=200, c='lightgray', edgecolors='black', linewidth=2, 
                          zorder=2)
            
            # Plot center point
            if frame['center'] is not None:
                center_coord = self.coords[frame['center']]
                ax.scatter(center_coord[0], center_coord[1], 
                          s=300, c='red', marker='*', edgecolors='darkred', 
                          linewidth=2, zorder=3)
                
                # Draw beta-radius circle
                circle = Circle(center_coord, self.beta, fill=False, 
                               edgecolor='red', linestyle='--', linewidth=2, 
                               zorder=1)
                ax.add_patch(circle)
            
            # Plot cluster points (Q) in color
            if frame['cluster']:
                cluster_coords = self.coords[frame['cluster']]
                ax.scatter(cluster_coords[:, 0], cluster_coords[:, 1], 
                          s=200, c='teal', edgecolors='darkgreen', 
                          linewidth=2, zorder=2)
            
            # Label all points with their indices directly on the points
            for i, (x, y) in enumerate(self.coords):
                ax.text(x, y, str(i), fontsize=10, ha='center', va='center', 
                       fontweight='bold', color='black')
            
            # Title and formatting
            if frame['center'] is not None:
                title = f"Step {frame['step'] + 1}: Center = {frame['center']}, |Q| = {len(frame['cluster'])}"
            else:
                title = f"Final: All clusters formed"
            
            ax.set_title(title, fontsize=14, fontweight='bold', pad=20)
            ax.set_aspect('equal')
            ax.grid(True, alpha=0.3)
            
            # Add completed clusters info and legend on the right
            ax_text.axis('off')
            
            # Build info text
            text_content = "Completed Clusters:\n\n"
            for i, cluster in enumerate(frame['completed_clusters']):
                text_content += f"Q{i+1} = {{{', '.join(map(str, sorted(cluster)))}}}\n"
            
            if frame['remaining']:
                text_content += f"\nRemaining R = {{{', '.join(map(str, sorted(frame['remaining'])))}}}\n"
            
            if frame['cluster']:
                text_content += f"\nCurrent Q = {{{', '.join(map(str, sorted(frame['cluster'])))}}}"
            
            ax_text.text(0.05, 0.95, text_content, transform=ax_text.transAxes, 
                        fontsize=10, verticalalignment='top', family='monospace',
                        bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
            
            # Create legend
            legend_elements = [
                mpatches.Patch(facecolor='lightgray', edgecolor='black', linewidth=2, label='R (Remaining)'),
                mpatches.Patch(facecolor='red', edgecolor='darkred', linewidth=2, label='Center'),
                mpatches.Patch(facecolor='none', edgecolor='red', linestyle='--', linewidth=2, label=f'β-radius = {self.beta}'),
                mpatches.Patch(facecolor='teal', edgecolor='darkgreen', linewidth=2, label='Q (Current)'),
                mpatches.Patch(facecolor='none', edgecolor='none', label=f'π = {self.permutation_str}')
            ]
            
            # Add legend on the right side below the text
            ax_text.legend(handles=legend_elements, loc='lower left', fontsize=8, 
                          frameon=True, bbox_to_anchor=(0, 0))
            
            # Add/update buttons only on first draw
            if is_first:
                ax_prev = self.fig.add_axes([0.2, 0.04, 0.08, 0.05])
                ax_next = self.fig.add_axes([0.72, 0.04, 0.08, 0.05])
                
                btn_prev = plt.Button(ax_prev, 'Previous', color='lightblue', hovercolor='0.975')
                btn_next = plt.Button(ax_next, 'Next', color='lightgreen', hovercolor='0.975')
                
                # Store reference to buttons and figure
                self.buttons['prev'] = (btn_prev, ax_prev)
                self.buttons['next'] = (btn_next, ax_next)
                
                # Use lambda to capture self reference properly
                btn_prev.on_clicked(lambda event: self._handle_prev())
                btn_next.on_clicked(lambda event: self._handle_next())
            
            # Update button states
            self.buttons['prev'][0].set_active(self.current_frame > 0)
            self.buttons['next'][0].set_active(self.current_frame < self.num_frames - 1)
            
            # Clear and update frame counter
            self.fig.texts.clear()
            counter_text = f"Frame {frame_idx + 1} of {self.num_frames}"
            self.fig.text(0.5, 0.01, counter_text, ha='center', fontsize=11, fontweight='bold')
            
            # Redraw the figure
            self.fig.canvas.draw_idle()
            
            # Make fullscreen on first draw
            if is_first:
                try:
                    mng = self.fig.canvas.manager
                    mng.window.state('zoomed')
                except:
                    try:
                        mng = self.fig.canvas.manager
                        mng.window.showMaximized()
                    except:
                        pass
        
        def _handle_prev(self):
            """Handle previous button click"""
            if self.current_frame > 0:
                self.current_frame -= 1
                self.draw_frame(self.current_frame)
        
        def _handle_next(self):
            """Handle next button click"""
            if self.current_frame < self.num_frames - 1:
                self.current_frame += 1
                self.draw_frame(self.current_frame)
        
        def _handle_key(self, event):
            """Handle keyboard shortcuts"""
            if event.key == 'left':
                self._handle_prev()
            elif event.key == 'right':
                self._handle_next()
        
        def on_prev(self, event):
            self._handle_prev()
        
        def on_next(self, event):
            self._handle_next()
    
    navigator = FrameNavigator(frames, coords, metric, beta, permutation_str)
    navigator.draw_frame(0)


# -------------------------
# User Input Section
# -------------------------
print("=" * 60)
print("PARTITION VISUALIZER - Frame by Frame")
print("=" * 60)

# Get metric input
metric_input = input("\nEnter metric points as R^2 coordinates (e.g., (0,1); (8,9); (5,3)): ").strip()
coords, metric = parse_metric_input(metric_input)

n = len(coords)
print(f"\nPoints parsed: {n}")
print(f"Coordinates:\n{coords}")
print(f"\nDistance Metric:\n{metric}")

# Calculate diameter (max interpoint distance)
max_distance = np.max(metric)
print(f"\n{'=' * 60}")
print(f"Max Interpoint Distance (Diameter): {max_distance:.4f}")
print(f"{'=' * 60}")

# Recommend beta range
beta_lower = max_distance / 8
beta_upper = max_distance / 4
print(f"\nRecommended β range: [{beta_lower:.4f}, {beta_upper:.4f}]")
print(f"  → diam(Q)/8 = {beta_lower:.4f}")
print(f"  → diam(Q)/4 = {beta_upper:.4f}")

# Get permutation input
permutation_input = input(f"\nEnter permutation (0 to {n-1}, separated by spaces or commas, e.g., 0 1 2 3): ").strip()
permutation_input = permutation_input.replace(",", " ")
permutation = list(map(int, permutation_input.split()))

# Get beta input with validation
while True:
    beta_input = input(f"\nEnter β (radius parameter) [recommended: {beta_lower:.4f} - {beta_upper:.4f}]: ").strip()
    try:
        beta = float(beta_input)
        if beta <= 0:
            print("⚠ β must be positive!")
            continue
        break
    except ValueError:
        print("⚠ Invalid input! Please enter a number.")

print("\n" + "=" * 60)
print("Generating frame-by-frame visualization...")
print("=" * 60)

# Visualize
visualize_partition_frames(coords, metric, permutation, beta)

plt.show()
