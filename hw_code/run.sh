for i in $(seq 1 1000); do
  # Only print output on the 1st iteration and every 25th iteration
  if (( i == 1 || i % 25 == 0 )); then
    echo "--- Run $i of 100 ---"
    ./tests
    # echo "---------------------"
    # echo "" # Add a blank line for readability
  else
    # Run the test but silence its output (stdout and stderr)
    ./tests
  fi
done
