# Travel Management System

A C++ desktop application built with **SFML 2.6.x** for managing travel packages, bookings, customer accounts, and admin operations.

> Course project — S26-12

---

## Features

- **Customer portal** — browse packages, book trips, design custom packages, leave reviews, subscribe to notifications
- **Admin portal** — add packages/cities, approve custom requests, broadcast notifications, view revenue report



---

## Requirements

- **Windows 10/11** (64-bit)
- **Visual Studio 2019 or 2022** 
  - Workload: *Desktop development with C++*
- **vcpkg** — manages SFML and curl automatically

---

## Setup & Run

### 1. Install vcpkg (one-time, skip if already installed)

```bash
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg integrate install
```

### 2. Clone this repo

```bash
git clone https://github.com/dandelionsum01/unofficial_1.git
cd unofficial_1
```

### 3. Copy arial.ttf

The app needs `arial.ttf` in the project root. On Windows it is already on your system:

```
Copy   C:\Windows\Fonts\arial.ttf   →   into the project folder (next to S26-12.vcxproj)
```

### 4. Open and build

- Open `S26-12.vcxproj` in Visual Studio
- Set configuration to **Debug x64** or **Release x64**
- Press **F5** — vcpkg will automatically download and link SFML + curl on the first build


---

