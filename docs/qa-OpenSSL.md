# OpenSSL 问题排查

## 错误信息

```
qt.network.ssl: QSslSocket::connectToHostEncrypted: TLS initialization failed
```

## 所需 DLL 文件

以下 DLL 文件需位于应用程序目录中：

- `libcrypto-1_1-x64.dll`
- `libgcc_s_seh-1.dll`
- `libssl-1_1-x64.dll`
