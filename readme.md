# 简单Web服务器

## 编译
```bash
make
```

## 压力测试
```bash
cd tools/test_presure/webbench-1.5
rm webbench && make
./webbench -c 5000 -t 5 http://xxx.xxx.xxx.xxx:xxxx/index.html
-c 客户端数量
-t 连接时间
```
### 测试结果
- 5000 clients, running 5 sec.

    Speed=310404 pages/min, 822379 bytes/sec.

    Requests: 25867 susceed, 0 failed.

- 8000 clients, running 5 sec.

    Speed=282888 pages/min, 749557 bytes/sec.

    Requests: 23574 susceed, 0 failed.

- 10000 clients, running 5 sec.

    Speed=327588 pages/min, 868108 bytes/sec.

    Requests: 27299 susceed, 0 failed.

- 12000 clients, running 5 sec.

    Speed=264324 pages/min, 700458 bytes/sec.

    Requests: 22027 susceed, 0 failed.

- 15000 clients, running 5 sec.

    Speed=343752 pages/min, 910942 bytes/sec.

    Requests: 28646 susceed, 0 failed.

- 20000 clients, running 5 sec.

    Speed=443136 pages/min, 1174246 bytes/sec.

    Requests: 36928 susceed, 0 failed.

- 30000 clients, running 5 sec.

    Speed=561072 pages/min, 1485409 bytes/sec.

    Requests: 46748 susceed, 8 failed.

- 10000 clients, running 10 sec.

    Speed=398184 pages/min, 1055187 bytes/sec.
    
    Requests: 66364 susceed, 0 failed.