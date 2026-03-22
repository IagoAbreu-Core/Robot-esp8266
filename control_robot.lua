local socket = require("socket")
local client = assert(socket.udp())
-- ip e porta do Robô
host = "192.168.1.10"
port = 1234

print("Comunicação com Robô")
local speed = 10000
local control = true
local cm = ""
local arq

while (1) do
	if control then
		print("M para motores DC, B para motores servo do Braço\nQuatro valores para mudar valor pwm de cada motor!")
		print("velocidade atual : "..speed)

		cm = io.read();

		if cm == "exit" then
			break
		elseif cm == "ler" then
			io.write("Digita nome do arquivo : ")
			cm = io.read()
			control = false
			arq = io.open(cm, "r")
		elseif cm == "speed" then
			io.write("Difine velocidade : ")
			speed = io.read("n")
		elseif cm == "stop" then
			cm = "M 0 0 0 0"
		elseif cm == "mup" then
			cm = "M "..speed.." 0 0 "..speed
		elseif cm == "mdown" then
			cm = "M 0 "..speed.." "..speed.." 0"
		elseif cm == "turnl" then
			cm = "M "..speed.." 0 "..speed.." 0"
		elseif cm == "turnr" then
			cm = "M 0 "..speed.." 0 "..speed
		end

	else
		cm = arq:read()
		if cm == "delay" then
			socket.sleep(1)
		elseif cm == nil then
			control = true
			arq:close()
			cm = "fim do script"
		end
	end

	assert(client:sendto(cm,host,port))

	client:settimeout(10)

	local response, err = client:receive()

	if response then
		print("Resposta do Servidor: " .. response)
	else
		print("Erro: " .. err)
	end

end

client:close()

