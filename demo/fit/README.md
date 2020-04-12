# fit

instance based game model to prove fitness of wise.kernel as a distributed game server. 
 

service:
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

 - game_directory_service
   - 게임 인스턴스들 관리 
   - 채널에 생성 요청


견교하고 단순하며 명확한 분산 시스템을 구성한다. 

suid 개념이 전체적인 구조를 단순하고 명확하게 만들었다. 
게임에 적합한 구조이다. 클라이언트도 actor로 포함했다. 
서버 입장에서는 모두 액터로 보고 클라이언트는 세션을 사용하여 송수신 하면 된다. 


