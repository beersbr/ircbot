require "socket"

# Don't allow use of "tainted" data by potentially dangerous operations... we are dealing with
# quite a few unknowns putting this on the webs
$SAFE=1

class ErcBot
    def initialize(server, port, nick, channel)
        @server = server
        @port = port
        @nick = nick
        @channel = channel

        @sticky = File.open("notepad.txt", "w+")
        @sticky.puts("------------------- #{Time.now.to_s} -------------------")

    end

    def clean()
        @sticky.close
    end

    def send(s)
        # Send a message to the irc server and print it to the screen
        puts "--> #{s}"
        @irc.send "#{s}\n", 0 
    end

    def connect()
        # Connect to the IRC server
        @irc = TCPSocket.open(@server, @port)
        send "USER blah blah blah :blah blah"
        send "NICK #{@nick}"
        send "JOIN #{@channel}"
    end

    def evaluate(s)
        # Make sure we have a valid expression (for security reasons), and
        # evaluate it if we do, otherwise return an error message
        if s =~ /^[-+*\/\d\s\eE.()]*$/ then
            begin
                s.untaint
                return eval(s).to_s
            rescue Exception => detail
                puts detail.message()
            end
        end
        return "Error"
    end

    def handle_input(s)
        case s.strip
            when /^.*:\\NOTE (.+)/i
                puts("noting:  #{$1}")
                send("PRIVMSG #{channel} :noted => #{$1}")
                @sticky.puts("#{Time.now.ctime}: #{$1}")
                @sticky.flush

            when /^PING :(.+)$/i
                puts "[ Server ping ]"
                send "PONG :#{$1}"
            when /^:(.+?)!(.+?)@(.+?)\sPRIVMSG\s.+\s:[\001]PING (.+)[\001]$/i
                puts "[ CTCP PING from #{$1}!#{$2}@#{$3} ]"
                send "NOTICE #{$1} :\001PING #{$4}\001"
            when /^:(.+?)!(.+?)@(.+?)\sPRIVMSG\s.+\s:[\001]VERSION[\001]$/i
                puts "[ CTCP VERSION from #{$1}!#{$2}@#{$3} ]"
                send "NOTICE #{$1} :\001VERSION Ruby-irc v0.042\001"
            when /^:(.+?)!(.+?)@(.+?)\sPRIVMSG\s(.+)\s:EVAL (.+)$/i
                puts "[ EVAL #{$5} from #{$1}!#{$2}@#{$3} ]"
                send "PRIVMSG #{(($4==@nick)?$1:$4)} :#{evaluate($5)}"
            else
                puts s
        end
    end
    
    def runner()
        while true
            ready = select([@irc, $stdin], nil, nil, nil)
            next if !ready
            for s in ready[0]
                if s == $stdin then
                    return if $stdin.eof
                    s = $stdin.gets
                    send s
                elsif s == @irc then
                    return if @irc.eof
                    s = @irc.gets
                    handle_input(s)
                end
            end
        end
    end
end

bot = ErcBot.new('chat.freenode.net', 6667, 'remmy', '#scrambles')
bot.connect()
begin
    bot.runner()
rescue Interrupt
rescue Exception => detail
    puts detail.message()
    print detail.backtrace.join("\n")
    retry
end

bot.clean