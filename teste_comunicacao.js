let port = null;
let writer = null;

document.getElementById('conectar').addEventListener('click', async () => {
    try {
        port = await navigator.serial.requestPort();
        await port.open({ baudRate: 115200 });
        writer = port.writable.getWriter();
        document.getElementById('status').textContent = 'Conectado';
    } catch (e) {
        console.error('Erro:', e);
    }
});

document.getElementById('testar').addEventListener('click', async () => {
    if (!writer) {
        alert('Conecte primeiro!');
        return;
    }
    
    try {
        const data = new TextEncoder().encode('teste\n');
        await writer.write(data);
        console.log('Comando enviado');
    } catch (e) {
        console.error('Erro:', e);
    }
});
