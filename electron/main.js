const { app, BrowserWindow, ipcMain, dialog } = require("electron");
const { spawn } = require("child_process");
const path = require("path");

let backend;
const isWindows = process.platform === 'win32';

function startBackend() {
  const exePath = app.isPackaged
    ? path.join(process.resourcesPath, "backend", "app.exe")
    : path.join(__dirname, "..", "backend", "app.exe");

  backend = spawn(exePath, [], {
    cwd: path.dirname(exePath),
    detached: false,
    stdio: "ignore",
  });

  backend.on('error', (err) => {
    console.error('Backend error:', err);
  });
}

function killBackend() {
  if (backend && !backend.killed) {
    if (isWindows) {
      spawn('taskkill', ['/pid', backend.pid, '/f', '/t']);
    } else {
      backend.kill('SIGTERM');
    }
    backend = null;
  }
}

ipcMain.handle("ask-input", async (_, message) => {
  return new Promise((resolve) => {
    const win = new BrowserWindow({
      width: 400,
      height: 200,
      modal: true,
      parent: BrowserWindow.getFocusedWindow(),
      webPreferences: {
        nodeIntegration: true,
        contextIsolation: false,
      },
    });

    win.loadFile(path.join(__dirname, "inputWindow.html"));

    win.webContents.on("did-finish-load", () => {
      win.webContents.send("set-message", message);
    });

    const listener = (_, value) => {
      resolve(value);
      ipcMain.removeListener("input-value", listener);
      win.close();
    };

    ipcMain.on("input-value", listener);
  });
});

function createWindow() {
  const win = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      nodeIntegration: false,
      contextIsolation: true,
    },
  });

  const indexPath = app.isPackaged
    ? `file://${path.join(process.resourcesPath, "frontend", "index.html")}`
    : `file://${path.join(__dirname, "..", "frontend", "index.html")}`;

  win.loadURL(indexPath);

  win.on("closed", () => {
    killBackend();
  });
}

app.whenReady().then(() => {
  startBackend();
  setTimeout(createWindow, 700);
});

app.on("window-all-closed", () => {
  killBackend();
  app.quit();
});

app.on('before-quit', () => {
  killBackend();
});