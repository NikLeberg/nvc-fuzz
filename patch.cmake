function(patch_once PATCH_FILE WORKING_DIRECTORY)
  execute_process(
    COMMAND git apply ${PATCH_FILE}
    WORKING_DIRECTORY ${WORKING_DIRECTORY}
    RESULT_VARIABLE PATCH_RESULT
    ERROR_QUIET
  )
  if(${PATCH_RESULT} GREATER 0)
    message(STATUS "Patch '${PATCH_FILE}' did not apply.")
  endif()
endfunction()

function(apply_upstream_prs GITHUB_USER REPO_OWNER REPO_NAME)
  set(PATCHES_DIR "${BIN}/upstream_patches")
  file(MAKE_DIRECTORY ${PATCHES_DIR})

  set(API_URL "https://api.github.com/search/issues?q=author:${GITHUB_USER}+type:pr+repo:${REPO_OWNER}/${REPO_NAME}+state:open")
  file(DOWNLOAD ${API_URL} ${PATCHES_DIR}/pr_list.json STATUS DOWNLOAD_STATUS)

  list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
  if(NOT STATUS_CODE EQUAL 0)
    message(STATUS "Failed to fetch upstream PRs from GitHub. Skipping.")
    return()
  endif()

  file(READ ${PATCHES_DIR}/pr_list.json PR_JSON)
  string(JSON PR_COUNT GET ${PR_JSON} "total_count")
  string(JSON ITEMS GET ${PR_JSON} "items")

  math(EXPR LAST_IDX "${PR_COUNT} - 1")
  foreach(IDX RANGE ${LAST_IDX})
    string(JSON PR_NUMBER GET ${ITEMS} ${IDX} "number")
    string(JSON PR_TITLE GET ${ITEMS} ${IDX} "title")

    file(DOWNLOAD
      "https://github.com/${REPO_OWNER}/${REPO_NAME}/pull/${PR_NUMBER}.patch"
      ${PATCHES_DIR}/${PR_NUMBER}.patch
    )

    message(STATUS "Fetched upstream PR #${PR_NUMBER}: '${PR_TITLE}'.")
    patch_once(${PATCHES_DIR}/${PR_NUMBER}.patch ${SRC})
  endforeach()
endfunction()

patch_once(${LST}/fatal_exit.patch ${SRC})
patch_once(${LST}/crash_dedup.patch ${SRC})

apply_upstream_prs("NikLeberg" "nickg" "nvc")
