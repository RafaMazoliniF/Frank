const { app, BrowserWindow } = require('electron')
const { spawn } = require('child_process')
const net = require('net')

let flaskProcess

function waitForFlask(port, callback) {
  const retry = () => {
    const socket = net.createConnection(port, '127.0.0.1')
    socket.on('connect', () => {
      socket.end()
      callback()
    })
    socket.on('error', () => {
      setTimeout(retry, 300)
    })
  }
  retry()
}

function createWindow() {
  const win = new BrowserWindow({
    width: 1000,
    height: 700
  })

  win.loadURL('http://127.0.0.1:5000')
}

app.whenReady().then(() => {
  flaskProcess = spawn('python', ['main.py'], {
    cwd: __dirname,
    stdio: 'inherit'
  })

  waitForFlask(5000, () => {
    createWindow()
  })
})

app.on('will-quit', () => {
  if (flaskProcess) flaskProcess.kill()
})
