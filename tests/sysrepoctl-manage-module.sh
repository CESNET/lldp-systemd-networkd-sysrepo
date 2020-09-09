#!/bin/bash

set -eux -o pipefail
shopt -s failglob

SYSREPOCTL="${1}"
shift
if [[ ! -x "${SYSREPOCTL}" ]]; then
  echo "Cannot locate \$SYSREPOCTL"
  exit 1
fi

SYSREPOCFG="${1}"
shift
if [[ ! -x "${SYSREPOCFG}" ]]; then
  echo "Cannot locate \$SYSREPOCFG"
  exit 1
fi

MODE="${1}"
shift
[ "${MODE}" != "prepare" ] && [ "${MODE}" != "uninstall" ] && error "Invalid mode of operation (neither prepare nor uninstall)"

while [[ $# -gt 0 ]]; do
  if [[ "${1}" == "YANG" ]]; then
    shift
  else
    error "Error: Expected 'YANG yangfile' clause"
  fi

  YANG_FILE="${1}"
  shift
  if [[ ! -f "${YANG_FILE}" ]]; then
    echo "Error: Specified YANG file does not exist"
    exit 1
fi

  MODULE=$(basename --suffix .yang "${YANG_FILE}" | sed 's/@.*//')  # also remove part after @ from module name
  YANG_DIR=$(dirname "${YANG_FILE}")

  if [[ "${MODE}" == "prepare" ]]; then
    ${SYSREPOCTL} --uninstall --module "${MODULE}" || true
    ${SYSREPOCTL} --install --yang "${YANG_FILE}"
  elif [[ "${MODE}" == "uninstall" ]]; then
    MODULE_LIST=( "${MODULE}" "${MODULE_LIST[@]}" ) # save for later; uninstall in reverse order
  fi

  # parse and process all arguments up to next "YANG" directive
  while [[ $# -gt 0 && ${1} != "YANG" ]]; do
    if [[ "${1}" == "FEATURE" ]]; then
      shift
      FEATURE="${1}"
      shift
      [ -z "${FEATURE}" ] && error "Error: FEATURE requires an argument"

      if [[ "${MODE}" == "prepare" ]]; then
        ${SYSREPOCTL} --module ${MODULE} --feature-enable ${FEATURE}
      fi
    elif [[ "${1}" == "JSON" || "${1}" == "XML" ]]; then
      FORMAT="${1}"
      shift
      DATA_FILE="${1}"
      shift
      if [[ "${MODE}" == "prepare" ]]; then
        if [[ ! -f "${DATA_FILE}" ]]; then
          echo "Error: ${FORMAT} data file ${DATA_FILE} does not exist"
          exit 1
        fi
        ${SYSREPOCFG} -d startup -f "${FORMAT,,}" "${MODULE}" -i "${DATA_FILE}"
      fi
    fi
  done
done

if [[ "${MODE}" == "uninstall" ]]; then
  for module in "${MODULE_LIST[@]}"; do
    ${SYSREPOCTL} --uninstall --module "${module}"
  done
fi