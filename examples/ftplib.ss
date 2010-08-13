(def ftp/new (fn (host port)
	((fn (sock)
		(fcntl 
