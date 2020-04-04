# fit

instance based game model to prove fitness of wise.kernel as a distributed game server. 

handler: 
 - don't follow actor model. use simpler and straightforward distribution
 - ref for location transparency
 - service is a root handler 

service:
 - root handler
 - peer_service
 - db_service
 - bot_service


to fit:
 - lobby_service
   - auth
   - match prep.
   - chat and other services
 - channel_service
   - game instances


이름을 무엇으로 할 지는 모르겠지만 다음과 같은 구조가 필요하다.

- host/handler/handler/.../handler
  - 이걸 뭐라 부를까?
- 클라이언트 접속과 이들을 참조하는 방법
  - host_1/client_1

클라이언트가 특별한가?  

견교하고 단순하며 명확한 분산 시스템을 구성한다. 

service
 - actor 
   - address
     - 1.3.5.9.11.3.10147
     - 1
     - 1.0
     - 1.3.0
   - child actors 
   - remote_ref
     - client_ref 
       - protocol
   - actor_ref
 - channel 
   - game 
   - player
     - client : remote_ref

actor는 이름만 빌려온다. 

어느 정도 정리되었다. 예전에 생각했던 것. 이와 같이 주소로 서로를 어디에 있건 아는 ref를 통해 전송을 할 수 있는 시스템을 구성한다. 하위 통신 시스템과 분리한다. 

bits IDL에 wise::hx로 만들었던 걸 재조정해서 정리한다. 
여기서 액터로 가건 다른 개념을 포함하건 한다. 



