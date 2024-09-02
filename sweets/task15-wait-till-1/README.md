### Statement

На стандартном потоке ввода программе задаются два целых числа.
Первое число представимо типом `time_t`, второе число - 32-битное
целое в интервале от 0 до 999999999. Эти числа в совокупности задают значение
типа `struct timespec`, определяющие момент времени наступления события (относительно начала эпохи Unix).
Программа должна с помощью системного вызова `setitimer`
настроить таймер реального времени, чтобы он сработал в
момент времени наступления события. Программа дожидается наступления события
и завершает свою работу с кодом 0.



Если момент наступления события оказался в прошлом, программа должна
завершиться немедленно.

    
