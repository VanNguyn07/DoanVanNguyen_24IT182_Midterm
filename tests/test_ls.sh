#!/bin/sh

check_dir=$(mktemp -d /tmp/ls-check.XXXXXX) || exit 1
trap 'rm -rf "$check_dir"' 0

passed=0
failed=0

run_case()
{
    label=$1
    shift

    LC_ALL=C /bin/ls -1 "$@" \
        > "$check_dir/system" 2> "$check_dir/system_error"
    system_status=$?

    LC_ALL=C ./ls "$@" \
        > "$check_dir/ours" 2> "$check_dir/our_error"
    our_status=$?

    if [ "$system_status" -eq "$our_status" ] &&
       diff -w -u "$check_dir/system" "$check_dir/ours" \
           > "$check_dir/difference"; then
        printf 'PASS: %s\n' "$label"
        passed=$((passed + 1))
    else
        printf 'FAIL: %s (system=%s, ours=%s)\n' \
            "$label" "$system_status" "$our_status"
        cat "$check_dir/difference"
        cat "$check_dir/our_error"
        failed=$((failed + 1))
    fi
}

run_case "basic listing" test_cases
run_case "hidden entries" -a test_cases
run_case "reverse and hidden" -Ar test_cases
run_case "inode and classification" -iF test_cases
run_case "long format" -l test_cases
run_case "numeric IDs and inode" -ni test_cases
run_case "blocks and KiB" -lsk test_cases
run_case "human sizes" -lsh test_cases
run_case "recursive with safe names" -Rq test_cases
run_case "recursive long format" -Rlq test_cases
run_case "directory operands" -di test_cases main.c
run_case "mixed operands" -l main.c test_cases .gitignore
run_case "size sort" -lS test_cases/sort_tests
run_case "reverse time sort" -ltr test_cases/sort_tests
run_case "access time sort" -ltu test_cases/sort_tests
run_case "status change time sort" -ltc test_cases/sort_tests
run_case "symbolic links" -lF test_cases/link_dir test_cases/broken_link
run_case "continue after error" main.c /thu_muc_khong_ton_tai .gitignore

printf '\nResult: %s PASS, %s FAIL\n' "$passed" "$failed"
[ "$failed" -eq 0 ]
