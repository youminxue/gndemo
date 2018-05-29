// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/test/python_utils.h"

#include <memory>

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/environment.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#if defined(OS_MACOSX)
#include "base/mac/foundation_util.h"
#endif

const char kPythonPathEnv[] = "PYTHONPATH";
const char kVPythonClearPathEnv[] = "VPYTHON_CLEAR_PYTHONPATH";

void ClearPythonPath() {
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  env->UnSetVar(kPythonPathEnv);

  // vpython has instructions on BuildBot (not swarming or LUCI) to clear
  // PYTHONPATH on invocation. Since we are clearing and manipulating it
  // ourselves, we don't want vpython to throw out our hard work.
  env->UnSetVar(kVPythonClearPathEnv);
}

void AppendToPythonPath(const base::FilePath& dir) {
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  std::string old_path;
  std::string dir_path;
#if defined(OS_WIN)
  dir_path = base::WideToUTF8(dir.value());
#elif defined(OS_POSIX)
  dir_path = dir.value();
#endif
  if (!env->GetVar(kPythonPathEnv, &old_path)) {
    env->SetVar(kPythonPathEnv, dir_path.c_str());
  } else if (old_path.find(dir_path) == std::string::npos) {
    std::string new_path(old_path);
#if defined(OS_WIN)
    new_path.append(";");
#elif defined(OS_POSIX)
    new_path.append(":");
#endif
    new_path.append(dir_path.c_str());
    env->SetVar(kPythonPathEnv, new_path);
  }
}

bool GetPyProtoPath(base::FilePath* dir) {
  // Locate the Python code generated by the protocol buffers compiler.
  base::FilePath generated_code_dir;
  if (!PathService::Get(base::DIR_EXE, &generated_code_dir)) {
    LOG(ERROR) << "Can't find " << generated_code_dir.value();
    return false;
  }

#if defined(OS_MACOSX)
  if (base::mac::AmIBundled())
    generated_code_dir = generated_code_dir.DirName().DirName().DirName();
#endif

  // Used for GYP. TODO(jam): remove after GN conversion.
  const base::FilePath kPyProto(FILE_PATH_LITERAL("pyproto"));
  if (base::DirectoryExists(generated_code_dir.Append(kPyProto))) {
    *dir = generated_code_dir.Append(kPyProto);
    return true;
  }

  // Used for GN.
  const base::FilePath kGen(FILE_PATH_LITERAL("gen"));
  if (base::DirectoryExists(generated_code_dir.Append(kGen))) {
    *dir = generated_code_dir.Append(kGen);
    return true;
  }

  return false;
}

bool GetPythonCommand(base::CommandLine* python_cmd) {
  DCHECK(python_cmd);

// Use vpython to pick up src.git's vpython VirtualEnv spec.
#if defined(OS_WIN)
  python_cmd->SetProgram(base::FilePath(FILE_PATH_LITERAL("vpython.bat")));
#else
  python_cmd->SetProgram(base::FilePath(FILE_PATH_LITERAL("vpython")));
#endif

  // Launch python in unbuffered mode, so that python output doesn't mix with
  // gtest output in buildbot log files. See http://crbug.com/147368.
  python_cmd->AppendArg("-u");

  return true;
}
