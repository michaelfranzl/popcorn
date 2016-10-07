"use strict";
/*
 * popcorn (c) 2016 Michael Franzl
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * boot.js
 * -------
 * 
 * 
 */

var msg = document.getElementById("msg");
var static_paths = API.getStaticPaths();
var fileread_jailed = API.getConfiguration("fileread_jailed") == "true";
var log_lines = [];
var paths = {
  home: static_paths.home_path,
  application: static_paths.application_path,
  working: API.getConfiguration("jail_working"),
}



function navigateTo(path) {
  var url = "file:///" + path + "assets/index.html";
  mylog("Navigating to", url);
  setTimeout(function() {
    location.href = url;
  }, 3000);
}

function relJailPathMaybeToAbsPath(jail_type, path_rel) {
  if (fileread_jailed) {
    return path_rel;
  } else {
    return paths[jail_type] + path_rel;
  }
}

function mylog() {
  var args = arguments;
  var line = "";
  var argarray = [];
  for ( var i = 0; i < args.length; i++ ) {
    if (typeof args[i] == "object"  ) {
      argarray.push( JSON.stringify(args[i], null, " ") );
    } else {
      argarray.push(args[i]);
    }
    line = argarray.join(" ");
  }
  API.printDebug("BOOT: " + line + "\n");
  log_lines.push(line); // add to end
  if (log_lines.length > 15) {
    log_lines.unshift(); // remove from beginning
  }
  msg.innerHTML = "<div>" + log_lines.join("<br/>") + "</div>";
}


function copyDirFromAppDirToWorkingDir(relpath) {
  var result;
  
  mylog("Removing", paths.working + relpath);
  result = API.dirRemove("working", relpath);
  mylog("Remove result:", result);

  mylog("Making dir", paths.working + relpath);
  result = API.dirMake("working", relpath);
  mylog("Making dir result:", result);
  
  var srcpath = relJailPathMaybeToAbsPath("application", relpath)
  var destpath_rel = relpath;
  mylog("Copying from", destpath_rel, "to", srcpath);
  result = API.dirCopy(destpath_rel, "working", srcpath, "application");
  mylog("Copying", result);
}


function versionStrToNumber(str) {
  return parseInt(str.replace(/\./g, ""));
}

function setupAssets() {
  var url;
  var awv = null; // assets version in jail_working directory
  var aav = null; // assets version in application directory
  
  if (API.fileExists("working", "assets/js/version.js")) {
    var str = API.fileRead("working", "assets/js/version.js");
    var matches = str.match(/assets_version = ['"](.+)["']/);
    if (matches) {
      awv = versionStrToNumber(matches[1]);
    }
    mylog("Found assets in working dir: Version ", matches[1]);
  }
  
  if (API.fileExists("application", "assets/js/version.js")) {
    var str = API.fileRead("application", "assets/js/version.js");
    var matches = str.match(/assets_version = ['"](.+)["']/);
    if (matches) {
      aav = versionStrToNumber(matches[1]);
    }
    mylog("Found assets in application dir: Version", matches[1]);
  }
  
  
  if (!awv && !aav) {
    mylog("Assets not found in either working directory (", jail_paths.working, ") or application directory (", paths.application, ").");
    
  } else  if (awv && !aav) {
    mylog("Assets only found in working directory.");
    url = paths.working;
    
  } else if (!awv && aav) {
    mylog("Assets only found in application directory.");
    copyDirFromAppDirToWorkingDir("assets");
    url = paths.working;
    
  } else if (awv > aav) {
    mylog("Upgraded assets found in working directory.");
    url = paths.working;
    
  } else if (awv == aav) {
    mylog("Assets in working and application directory are the same.");
    url = paths.working;

  } else {
    mylog("Newer assets found in application directory.");
    copyDirFromAppDirToWorkingDir("assets");
    url = paths.working;
  }
  return url;
}

/**
 * Copy newer versions of plugins from app dir to working dir.
 */
function setupPlugins() {

  mylog("Setting up plugins");
  var plugin_dirs = API.ls("application", "plugins", "*");
  
  for (var j = 0; j < plugin_dirs.length; j++) {
    var plugin_dir = plugin_dirs[j];
    var plugin_name = plugin_dir;
    var regexp = new RegExp("plugin_" + plugin_name + "_version = ['\"](.+)[\"']");
    
    var version_str_app = API.fileRead("application", "plugins/" + plugin_dir + "/version.txt");
    var version_appdir;
    if (version_str_app) {
      version_appdir = versionStrToNumber(version_str_app);
      mylog("Plugin", plugin_name, "in app dir:", version_appdir);
    } else {
      mylog("Plugin", plugin_name, "in app dir: Could not find version. Skipping.");
      continue;
    }
    
    var versionfile_workingdir = "plugins/" + plugin_dir + "/version.txt";
    var version_str_working = API.fileRead("working", versionfile_workingdir);
    var version_workingdir;
    if (version_str_working) {
      version_workingdir = versionStrToNumber(version_str_working);
      mylog("Plugin", plugin_name, "in working dir:", version_workingdir);
    }
    
    if (!version_workingdir || version_appdir > version_workingdir) {
      mylog("Plugin", plugin_name, "copying.");
      copyDirFromAppDirToWorkingDir("plugins/" + plugin_dir);
    } else {
      mylog("Plugin", plugin_name, "up to date or newer");
    }
  }
}

mylog(API.getAppName(), API.getVersion());
mylog("Booting...");
var boot_from = setupAssets();
if (boot_from) {
  setupPlugins();
  navigateTo(boot_from);
}