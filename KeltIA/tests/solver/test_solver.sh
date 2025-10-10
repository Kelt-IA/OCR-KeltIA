#!/bin/bash
# this sccript is meant to be called from the makefile
# with the command `make test-solver`

GRID_PATH="tests/solver/grid"
SOLVER="./solver"

$SOLVER $GRID_PATH horizontal
$SOLVER $GRID_PATH vertical
$SOLVER $GRID_PATH diagonal
$SOLVER $GRID_PATH find
$SOLVER $GRID_PATH hello
$SOLVER $GRID_PATH world
$SOLVER $GRID_PATH goldorak
$SOLVER $GRID_PATH epita

