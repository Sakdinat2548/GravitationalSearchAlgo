#!/usr/bin/env python3
"""
Interactive GSA Visualizer
- Navigate through iterations with next/previous buttons
- Hover/click to see particle fitness and mass
- Highlight the best particle in each iteration
- Show convergence history
"""

import argparse
from pathlib import Path
from typing import Optional

import numpy as np
import pandas as pd

# Try streamlit first, fall back to matplotlib
try:
    import streamlit as st
    HAS_STREAMLIT = True
except ImportError:
    HAS_STREAMLIT = False

if HAS_STREAMLIT:
    import plotly.graph_objects as go
    import plotly.express as px


def load_data(csv_path: Path) -> pd.DataFrame:
    """Load and validate particle CSV data."""
    if not csv_path.exists():
        raise FileNotFoundError(f"CSV file not found: {csv_path}")
    
    df = pd.read_csv(csv_path)
    required = {"iteration", "agent", "x", "y", "mass", "fitness"}
    missing = required.difference(df.columns)
    if missing:
        raise ValueError(f"CSV missing required columns: {sorted(missing)}")
    return df


def streamlit_app(df: pd.DataFrame) -> None:
    """Main Streamlit interactive visualization."""
    st.set_page_config(page_title="GSA Visualizer", layout="wide")
    st.title("🌌 Gravitational Search Algorithm Visualizer")
    
    frames = sorted(df["iteration"].unique())
    
    # Session state for current frame
    if "current_frame_idx" not in st.session_state:
        st.session_state.current_frame_idx = 0
    
    # Control panel
    col1, col2, col3, col4 = st.columns(4)
    with col1:
        if st.button("◀ Previous", use_container_width=True, key="btn_prev"):
            st.session_state.current_frame_idx = max(0, st.session_state.current_frame_idx - 1)
            st.rerun()
    with col2:
        if st.button("Next ▶", use_container_width=True, key="btn_next"):
            st.session_state.current_frame_idx = min(len(frames) - 1, st.session_state.current_frame_idx + 1)
            st.rerun()
    with col3:
        slider_val = st.slider(
            "Iteration",
            0,
            len(frames) - 1,
            st.session_state.current_frame_idx,
            key="iteration_slider",
        )
        if slider_val != st.session_state.current_frame_idx:
            st.session_state.current_frame_idx = slider_val
            st.rerun()
    with col4:
        st.write(f"**Frame {st.session_state.current_frame_idx} / {len(frames) - 1}**")
    
    current_iter = frames[st.session_state.current_frame_idx]
    frame_df = df[df["iteration"] == current_iter].copy()
    
    # Main visualization columns
    viz_col, stats_col = st.columns([3, 1])
    
    with viz_col:
        # Create Plotly scatter plot with labels
        fig = go.Figure()
        
        # Get iteration-specific data
        max_mass = frame_df["mass"].max()
        best_idx = frame_df["fitness"].idxmin()
        best_row = frame_df.loc[best_idx]
        
        # Add particle trail if multiple frames
        if st.session_state.current_frame_idx > 0:
            history = df[df["iteration"] <= current_iter]
        else:
            history = frame_df
        
        # Plot all particles with custom text
        for _, row in frame_df.iterrows():
            mass_norm = row["mass"] / max_mass if max_mass > 0 else 0.5
            size = 10 + 40 * mass_norm
            
            is_best = row["fitness"] == best_row["fitness"]
            color = "red" if is_best else "blue"
            
            label = (
                f"Agent {int(row['agent'])}<br>"
                f"Fitness: {row['fitness']:.6e}<br>"
                f"Mass: {row['mass']:.4f}<br>"
                f"Pos: ({row['x']:.2f}, {row['y']:.2f})"
            )
            
            fig.add_trace(
                go.Scatter(
                    x=[row["x"]],
                    y=[row["y"]],
                    mode="markers+text",
                    marker=dict(
                        size=size,
                        color=color,
                        opacity=0.7,
                        line=dict(width=2, color="black") if is_best else dict(width=0),
                    ),
                    text=[f"A{int(row['agent'])}"],
                    textposition="top center",
                    textfont=dict(size=8),
                    hovertext=label,
                    hoverinfo="text",
                    showlegend=False,
                )
            )
        
        # Add particle trails
        if st.session_state.current_frame_idx > 0:
            for agent_id in frame_df["agent"].unique():
                history_agent = df[
                    (df["agent"] == agent_id) & (df["iteration"] <= current_iter)
                ]
                fig.add_trace(
                    go.Scatter(
                        x=history_agent["x"],
                        y=history_agent["y"],
                        mode="lines",
                        line=dict(color="rgba(100,100,100,0.3)", width=1),
                        hoverinfo="skip",
                        showlegend=False,
                    )
                )
        
        # Configure layout
        x_min, x_max = frame_df["x"].min(), frame_df["x"].max()
        y_min, y_max = frame_df["y"].min(), frame_df["y"].max()
        padding = 0.5
        
        fig.update_layout(
            title=f"Iteration {current_iter} (Best Fitness: {best_row['fitness']:.6e})",
            xaxis=dict(
                range=[x_min - padding, x_max + padding],
                title="X",
                showgrid=True,
            ),
            yaxis=dict(
                range=[y_min - padding, y_max + padding],
                title="Y",
                showgrid=True,
            ),
            height=600,
            hovermode="closest",
            plot_bgcolor="rgba(240,240,240,1)",
        )
        
        st.plotly_chart(fig, use_container_width=True)
    
    with stats_col:
        st.subheader("Statistics")
        st.metric("Best Fitness", f"{best_row['fitness']:.6e}")
        st.metric("Best Mass", f"{best_row['mass']:.4f}")
        st.metric("Particles", len(frame_df))
        
        # Show top 5 particles
        st.subheader("Top 5 Particles")
        top5 = frame_df.nsmallest(5, "fitness")[
            ["agent", "fitness", "mass"]
        ].copy()
        top5.columns = ["Agent", "Fitness", "Mass"]
        st.dataframe(top5, use_container_width=True)
    
    # Convergence history plot
    st.subheader("Convergence History")
    convergence = df.groupby("iteration")["fitness"].min()
    fig_conv = go.Figure()
    fig_conv.add_trace(
        go.Scatter(
            x=convergence.index,
            y=convergence.values,
            mode="lines+markers",
            name="Best Fitness",
            line=dict(color="darkblue", width=2),
        )
    )
    fig_conv.add_vline(
        x=current_iter,
        line_dash="dash",
        line_color="red",
        annotation_text="Current",
    )
    fig_conv.update_layout(
        title="Best Fitness Over Iterations",
        xaxis_title="Iteration",
        yaxis_title="Fitness Value",
        height=300,
        hovermode="x unified",
    )
    st.plotly_chart(fig_conv, use_container_width=True)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Interactive GSA particle visualization with Streamlit"
    )
    parser.add_argument(
        "--input",
        "-i",
        type=Path,
        default=Path("gsa_particles_2d.csv"),
        help="Path to CSV particle data",
    )
    parser.add_argument(
        "--port",
        "-p",
        type=int,
        default=8501,
        help="Streamlit port (default: 8501)",
    )
    args = parser.parse_args()
    
    if not HAS_STREAMLIT:
        print("Error: Streamlit not installed.")
        print("Install with: pip install streamlit plotly")
        return
    
    df = load_data(args.input)
    streamlit_app(df)


if __name__ == "__main__":
    if HAS_STREAMLIT:
        main()
    else:
        print("Error: Streamlit is required. Install with: pip install streamlit plotly")
