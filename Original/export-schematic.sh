#!/bin/bash

# Define paths
PROJECT_PATH="."
SCHEMATIC_FILE="${PROJECT_PATH}/Epiphone Blues Custom 30.kicad_sch"
OUTPUT_DIR="${PROJECT_PATH}/outputs"

echo $SCHEMATIC_FILE

# Run KiCad CLI commands to export schematic to PDF and SVG
mkdir -p "${OUTPUT_DIR}"
kicad-cli sch export pdf -o "${OUTPUT_DIR}/schematic.pdf" "${SCHEMATIC_FILE}"
kicad-cli sch export svg -o "${OUTPUT_DIR}/schematic.svg" "${SCHEMATIC_FILE}"

