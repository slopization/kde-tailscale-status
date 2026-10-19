# kde-tailscale-status

KDE Plasma 바탕화면에 지정된 Tailscale 장비의 연결 상태(연결됨/연결해제)를 보여주는 Plasma plasmoid(바탕화면 위젯) 프로젝트.

계획은 [docs/PLAN.md](docs/PLAN.md) 참조.

## 개발 환경

- Ubuntu 26.04, Plasma 6.6.6, Tailscale 1.104.1

## 빌드와 테스트

```
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## 설치

```
sudo cmake --install build
kpackagetool6 -t Plasma/Applet -i package
```

`cmake --install`이 QML module을 Qt import path에 넣고, plasmoid package는 애드위젯 picker에 나타난다.

개발 중 빠른 test(plasma-sdk 필요):

```
QML2_IMPORT_PATH=build/bin plasmoidviewer -a package --location=floating --formfactor=planar
```
