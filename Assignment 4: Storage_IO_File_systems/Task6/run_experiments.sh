#!/bin/bash

# Task 6: File Directories - Soft and Hard Links Experiment
# This script performs all the experiments required for the assignment

echo "========================================="
echo "Task 6: File Directories Experiments"
echo "========================================="
echo ""

# Clean up any existing files from previous runs
rm -f file_original.txt file_soft.txt file_origin2.txt file_hard.txt

echo "========================================="
echo "PART A: SOFT LINK EXPERIMENTS"
echo "========================================="
echo ""

# Create original file
echo "Step 1: Creating file_original.txt with random text"
echo "This is the original file with some random text content for testing purposes." > file_original.txt
echo "Command: echo 'This is the original file with some random text content for testing purposes.' > file_original.txt"
echo "File created successfully!"
echo ""

# (a) Create soft link
echo "========================================="
echo "(a) Creating soft link"
echo "========================================="
echo "Command used: ln -s file_original.txt file_soft.txt"
ln -s file_original.txt file_soft.txt
echo "Soft link created successfully!"
echo ""

# (b) Check inodes
echo "========================================="
echo "(b) Checking inode numbers"
echo "========================================="
echo "Command to find inode: ls -li <filename>"
echo ""
echo "Inode information:"
ls -li file_original.txt file_soft.txt
echo ""
echo "Analysis:"
echo "- file_original.txt inode: $(ls -i file_original.txt | awk '{print $1}')"
echo "- file_soft.txt inode: $(ls -i file_soft.txt | awk '{print $1}')"
echo ""

# (c) Edit soft link and check original
echo "========================================="
echo "(c) Editing file_soft.txt"
echo "========================================="
echo "Adding text via soft link..."
echo "Added line via file_soft.txt" >> file_soft.txt
echo ""
echo "Contents of file_soft.txt:"
cat file_soft.txt
echo ""
echo "Contents of file_original.txt:"
cat file_original.txt
echo ""

# (d) Delete original and try to access soft link
echo "========================================="
echo "(d) Deleting file_original.txt"
echo "========================================="
echo "Command: rm file_original.txt"
rm file_original.txt
echo "file_original.txt deleted!"
echo ""
echo "Checking file_soft.txt status:"
ls -li file_soft.txt
echo ""
echo "Attempting to read file_soft.txt:"
cat file_soft.txt 2>&1 || echo "Error: Cannot access file_soft.txt"
echo ""
echo "Attempting to edit file_soft.txt:"
echo "New content" >> file_soft.txt 2>&1 || echo "Error: Cannot edit file_soft.txt"
echo ""

echo "========================================="
echo "PART B: HARD LINK EXPERIMENTS"
echo "========================================="
echo ""

# Create second original file
echo "Step 2: Creating file_origin2.txt with random text"
echo "This is the second original file with different random text for hard link testing." > file_origin2.txt
echo "Command: echo 'This is the second original file with different random text for hard link testing.' > file_origin2.txt"
echo "File created successfully!"
echo ""

# (e) Create hard link
echo "========================================="
echo "(e) Creating hard link"
echo "========================================="
echo "Command used: ln file_origin2.txt file_hard.txt"
ln file_origin2.txt file_hard.txt
echo "Hard link created successfully!"
echo ""

# (f) Check inodes for hard link
echo "========================================="
echo "(f) Checking inode numbers for hard link"
echo "========================================="
echo "Inode information:"
ls -li file_origin2.txt file_hard.txt
echo ""
echo "Analysis:"
echo "- file_origin2.txt inode: $(ls -i file_origin2.txt | awk '{print $1}')"
echo "- file_hard.txt inode: $(ls -i file_hard.txt | awk '{print $1}')"
echo "- Link count: $(ls -l file_hard.txt | awk '{print $2}')"
echo ""

# (g) Edit hard link and check both files
echo "========================================="
echo "(g) Editing file_hard.txt"
echo "========================================="
echo "Adding text via hard link..."
echo "Added line via file_hard.txt" >> file_hard.txt
echo ""
echo "Contents of file_hard.txt:"
cat file_hard.txt
echo ""
echo "Contents of file_origin2.txt:"
cat file_origin2.txt
echo ""
echo "Checking inodes after edit:"
ls -li file_origin2.txt file_hard.txt
echo ""
echo "Analysis:"
echo "- file_origin2.txt inode: $(ls -i file_origin2.txt | awk '{print $1}')"
echo "- file_hard.txt inode: $(ls -i file_hard.txt | awk '{print $1}')"
echo ""

# (h) Delete original and try to access hard link
echo "========================================="
echo "(h) Deleting file_origin2.txt"
echo "========================================="
echo "Command: rm file_origin2.txt"
rm file_origin2.txt
echo "file_origin2.txt deleted!"
echo ""
echo "Checking file_hard.txt status:"
ls -li file_hard.txt
echo "- Link count after deletion: $(ls -l file_hard.txt | awk '{print $2}')"
echo ""
echo "Attempting to read file_hard.txt:"
cat file_hard.txt
echo ""
echo "Attempting to edit file_hard.txt:"
echo "New line added after deletion" >> file_hard.txt
echo "Edit successful! Contents:"
cat file_hard.txt
echo ""

echo "========================================="
echo "All experiments completed successfully!"
echo "========================================="
