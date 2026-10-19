# 기획

## 목적

Plasma 바탕화면 위젯(plasmoid)으로 지정된 Tailscale 장비의 연결 상태(연결됨/연결해제)를 보여준다. MVP는 "지정 장비 목록 + 상태 표시등"이다.

## 요구사항

1. 설정에서 watching할 장비를 지정한다(단일 또는 복수)
2. 장비별 상태: active/inactive 텍스트로 표시. tailscaled 자체 상태 이상 시 별도 상태 표시
3. 자동 갱신. 주기는 설정 가능
4. 패널이 아니라 바탕화면에 배치되는 위젯

## 데이터 계층

`tailscale status --json`은 `ipnstate.Status`의 JSON ([ipnstate.go](https://github.com/tailscale/tailscale/blob/main/ipn/ipnstate/ipnstate.go)). CLI 자체가 포맷이 릴리스 간에 바뀌었다고 경고하므로, 파싱은 필요한 최소 필드만 가정하고 방어적으로 처리한다.

상태 판정 규칙:

- 연결해제: peer의 `Online`이 false. `Online`은 컨트롤 플레인 생존성이고 데이터 패스 연결이 아니다. `LastSeen`은 offline일 때만 존재
- 연결됨: `Online`이 true. `Active`는 "최근 2분 내 패킷 전송"이라는 느슨한 정의(코드에서 정의가 불명확하고 바뀔 수 있다고 명시)로, 보조 표시로만 사용한다
- 물리 네트워크가 죽어도 `BackendState`는 Running을 유지한다. 이런 경우는 `Health` 배열을 보조 상태로 표시
- 장비 식별: `HostName`은 고유하지 않을 수 있다(documented). `ID`(StableNodeID) 또는 `DNSName`으로 키를 잡는다. 설정 입력은 HostName도 허용하고 내부적으로 ID로 해석
- Latency는 status JSON에 없다. `tailscale ping`에만 있으므로 MVP에 포함하지 않는다
- `--active` flag는 inactive peer를 JSON output에서 제외한다. 연결해제 표시가 목적이므로 `--json`만 사용한다

갱신 방식: MVP는 폴링. LocalAPI unix socket 조회이고 in-memory 상태 assembly라 비용이 작아 수 초 간격 폴링이 실용적이다(공식 권장 주기는 문서화된 것 없음). 이벤트 드리븐 upgrade 경로: `client/local`의 `WatchIPNBus`(`/localapi/v0/watch-ipn-bus`, 숨은 CLI 명령 `tailscale debug watch-ipn`). Webhooks는 online/offline 이벤트를 커버하지 않는다([issue #5967](https://github.com/tailscale/tailscale/issues/5967) open).

## 표현 계층

Plasma 6 확인 결과:

- old pure JS applet(declarativescripts)은 Plasma 6에서 로드되지 않는다. Data engine은 kf6에서 deprecated이고 Plasma5Support compat framework에 남아 있다([Plasma5Support README](https://github.com/KDE/plasma5support), [Akademy 2023 post](https://notmart.org/blog/2023/07/akademy-2023-plasma-6-and-plasmoids/)). 지원 방식은 QML-only plasmoid 또는 QML + C++ plugin([porting guide](https://develop.kde.org/docs/plasma/widget/porting_kf6/))
- metadata.json에 stdin/stdout로 대화하는 helper를 선언하는 Plasma mechanism은 없다. CLI 실행 precedent는 QProcess([executable data engine 구현](https://github.com/KDE/plasma5support/blob/master/src/dataengines/executable/executable.cpp))
- Run path: C++ QML module + QProcess. C++ plasmoid는 compile이 필요하고 KDE store에 distribution할 수 없다([C API docs](https://develop.kde.org/docs/plasma/widget/c-api/)). deprecated 대안은 QML-only + Plasma5Support "executable" data engine이고 이 프로젝트에서는 사용하지 않는다
- 바탕화면 위젯을 만드는 별도 metadata field는 없다. Desktop containment는 plasmoid를 받고, Category는 widget list filter string일 뿐([properties docs](https://develop.kde.org/docs/plasma/widget/properties/)). Picker에 나타나려면 X-Plasma-API-Minimum-Version "6.0"이 필요하다
- 설치 위치: ~/.local/share/plasma/plasmoids/<pluginId>/. kpackagetool6로 설치하고 plasmoidviewer --formfactor=planar로 테스트한다([setup](https://develop.kde.org/docs/plasma/widget/setup/), [testing](https://develop.kde.org/docs/plasma/widget/testing/))

같은 형태의 precedent: [DeadIndian/tailscale-widget](https://github.com/DeadIndian/tailscale-widget)(C++ QProcess backend), [stackrot/PlasmaScale](https://github.com/stackrot/PlasmaScale)(QML-only, store distribution), [reality2-roycdavies/kde-tailscale](https://github.com/reality2-roycdavies/kde-tailscale). 이 프로젝트의 차별점은 위 상태 판정 규칙에 따른 지정 장비 전용 상태 표시등이다.

## 디렉터리 구조

```
package/
  metadata.json
  contents/ui/main.qml
  contents/config/
src/
CMakeLists.txt
tests/
```

## 마일스톤

1. 데이터 계층: status JSON fetch, parse, 상태 mapping 함수. 단위 테스트는 이 부분에 집중
2. UI: 장비별 원 표시등과 active/inactive 텍스트, 이름은 Tailscale registered name(DNSName의 first label, 없으면 HostName), 최소 설정(장비 목록, 폴링 주기)
3. 설치: plasmoid 디렉터리 배치와 애드위젯 picker에서 확인
4. 선택적 upgrade: WatchIPNBus 기반 이벤트 드리븐 갱신

## 리스크

- status JSON 필드 변화: 가정 최소화, parse 실패 시 명확히 실패 표시
- Active 정의 변화: UI 문구가 "지금 Reachable" 같은 단정적 정의에 의존하지 않게 처리
- 중복 장비명: HostName 기준 매칭은 ambiguous, ID 또는 DNSName으로 해결
