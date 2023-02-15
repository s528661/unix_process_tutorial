# -*- coding : utf-8 -*-
###############################################################################
##
## sdengd 簡易伝言板サーバ()
##
###############################################################################
$destruction_keyword = "valsu" # サーバ消滅用キーワード
$N_MEMO = 10 # 記録できる伝言の数
## デーモン化のためのおまじない
trap(:HUP,nil)
if (pid = fork())
    printf("PID: %d\n",pid)
    exit(0)
end
Dir::chdir("/tmp")
Process::setsid()
ObjectSpace::each_object(IO) { |io| io.close() unless io.closed?() }
###############################################################################
## ソケット通信の準備
###############################################################################
## C 言語でいうところの # include <... >
require "socket" # ソケット通信用ライブラリ
require "thread" # マルチスレッド用ライブラリ
host = "10.0.2.15"
# 自分のホストネームを取得
port = 40000 # 伝言板サービスは 40000 番
$entry = TCPServer::open(host,port) # 40000 番に受け付け窓口を開く
## 伝言関連の初期化
$memo_date = [] # 記録時間保存用配列
$memo_from = [] # 発信元保存用配列
$memo_pass = [] # 伝言削除用パスワード配列
$memo_text = [] # 伝言保存用配列
$memo_lock = [] # 伝言ロック用ミューテックス配列
for n in (0 ... $N_MEMO)
    $memo_date [n] = nil
    $memo_from [n] = nil
    $memo_pass [n] = nil
    $memo_text [n] = nil
    $memo_lock [n] = Mutex::new()
end
###############################################################################
## 伝言板サーバの通信処理
###############################################################################
while (true)
    Thread::start($entry.accept()) do |sock|
    ## $entry から sock に個々のクライアントの接続を
    ## 引き継ぎ、専用スレッドを割り当てて実行する
    ## 接続ユーザ名取得
    from = sock.gets().chomp() + "@" + sock.peeraddr()[2]
    ## 接続に成功したら、クライアントに OK を返す
    sock.puts("OK")
    while (line = sock.gets()) # クライアントから１行読み込み
    args = line.split(" ") # 空白で分割して配列に代入
    op = args [0]
    if (op == "put")
        ## put リクエストの処理 #######################
        n = args[1].to_i() # args [1] を整数に変換
        pass = args[2] # args [2] はパスワード
        if (n < 0 || $N_MEMO <= n)
            ## n が範囲外なら ERR を返す
            sock.puts("ERR")
        elsif ($memo_text[n] == nil && $memo_lock[n].locked? == false)
            ## 番号 n の伝言が nil
            $memo_date[n] = Time::now()
            $memo_from[n] = from
            $memo_pass[n] = pass
            text = ""
            while (line = sock.gets())
                ## クライアントからの入力を
                ## ピリオドのみの行まで読む
                if (line == ".\n")
                    break
                end
                text += line
            end
            $memo_text[n] = text
            sock.puts("OK")
        else
            ## 伝言が nil でない。
            sock.puts("ERR")
        end
    elsif (op == "get")
        ## get リクエストの処理 #######################
        n = args[1].to_i() # args [1] を整数に変換
        date_n = $memo_date[n]
        from_n = $memo_from[n]
        text_n = $memo_text[n]
        if (n < 0 || $N_MEMO <= n)
            ## n が範囲外なら ERR を返す
            sock.puts("ERR")
        elsif ( date_n && text_n )
            ## 伝言が nil でないなら
            ## クライアントに送信
            sock.puts("OK")
            sock.print("Date :")
            sock.puts(date_n.to_s())
            sock.print("From :")
            sock.puts(from_n)
            sock.puts("\n")
            sock.puts(text_n)
            sock.puts(".")
        else
            sock.puts("ERR")
        end
    elsif (op == "del")
        ## del リクエストの処理 #######################
        n = args[1].to_i() # args [1] を整数に変換
        pass = args[2] # args [2] はパスワード
        if (n < 0 || $N_MEMO <= n)
            ## n が範囲外なら ERR を返す
            sock.puts("ERR")
        elsif ($memo_text[n])
            if (pass == $memo_pass[n])
                if ( $memo_lock[n].locked? == false )
                    $memo_lock [n].synchronize {
                        ## ロックの解除
                        $memo_date[n] = nil
                        $memo_from[n] = nil
                        $memo_pass[n] = nil
                        $memo_text[n] = nil
                        sock.puts("OK")
                    }
                else
                    sock.puts("ERR")
                end
            else
                sock.puts("ERR")
            end
        else
            sock.puts("ERR")
        end
    elsif (op == "list")
        ## list リクエストの処理 ######################
        sock.puts("OK")
        for n in (0 ... $N_MEMO)
            date_n = $memo_date[n]
            from_n = $memo_from[n]
            text_n = $memo_text[n]
            if (date_n && text_n)
                lines = text_n.split("\n")
                sock.printf("%02d:",n)
                sock.puts(lines[0].to_s())
            end
        end
        sock.puts(".")
    elsif (op == "quit")
        ## quit リクエストの処理 ######################
        break
    elsif (op == $destruction_keyword)
        ## 自己消滅命令の処理 #########################
        sock.puts("OK")
        sock.puts("機密保護のため、自己消滅します。")
        sleep(2)
        sock.puts("Good luck !")
        sock.puts(".")
        sleep(2)
        exit(0)
    else
        ## その他の処理 ###############################
        sock.puts("ERR")
    end
end
sock.close()
end
end
###############################################################################
