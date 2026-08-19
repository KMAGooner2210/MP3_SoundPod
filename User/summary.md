# Summary

## Dự án tạo 5 task

###  vTaskAudio

* Priority: 4

* Stack: 128 words

* Vai trò: Quản lý trạng thái audio và DFPlayer

###  vTaskButton

* Priority: 3

* Stack: 128 words

* Vai trò: Quét phím và phát lệnh 

###  vTaskVisualizer

* Priority: 3

* Stack: 256 words

* Vai trò: Đọc ADC, tính biên độ, cập nhật WS2812


###  vTaskOled

* Priority: 2

* Stack: 256 words

* Vai trò: Vẽ giao diện TFT

###  vTaskLed

* Priority: 1

* Stack: 64 words

* Vai trò: Heartbeat LED PC13

##  FreeRTOS

### Task

* Task là 1 luồng xử lý độc lập về mặt logic 

* Mỗi task phụ trách một công việc và chạy trong vòng lặp vô hạn

* Mục đích:

	* Thay vì viết mọi chức năng trong 1 vòng while(1), mỗi chức năng được đặt trong 1 task riêng
	
	* Giúp các chức năng hoạt động gần như đồng thời
	
	* Một chức năng không phải quản lý toàn bộ timing của chức năng khác
	
	* Lỗi hoặc thay đổi trong một module ít ảnh hưởng đến module khác

### Priority

* Thể hiện mức ưu tiên thực thi của task

* Khi nhiều task cùng ở trạng thái sẵn sàng chạy, scheduler sẽ chọn task có priority cao hơn

* Một task priority cao đang ở trạng thái blocked vẫn nhường CPU cho task thấp hơn

### Scheduler

* Theo dõi trạng thái các task

* Chọn task nào được chạy

* Chuyển CPU từ task này sang task khác

* Ưu tiên task có priority cao hơn khi task đó sẵn sàng

### Queue

* Queue là cơ chế truyền dữ liệu an toàn giữa các task

* Trong dự án có 2 luồng queue chính

	* Audio Queue
	
		* Dữ liệu truyền qua queue là các lệnh: Next, Previous, Play/Pause, Volume Up, Volume Down
		
		* Button Task không trực tiếp điều khiển DFPlayer mà nó chỉ tạo sự kiện và gửi vào queue  

				Button Task  
					 |  
					 | command  
					 v  
				Audio Queue  
					 |  
					 v  
				Audio Task      

	* Display Queue
	
		* Dữ liệu gửi sang màn hình gồm: Bài hát hiện tại, tổng số bài, trạng thái, âm lượng
		
		* Button Task không trực tiếp điều khiển DFPlayer mà nó chỉ tạo sự kiện và gửi vào queue  

					Audio Task  
						  |  
						  | trạng thái player  
						  v  
					Display Queue  
						  |  
						  v  
					Display Task

### Blocking

* Blocking nghĩa là task tạm thời không cần CPU vì đang chờ

	* Dữ liệu từ Queue
	
	* Hết thời gian delay
	
	* Một sự kiện nào đó

* VD:

				Không có command  
						|  
						v  
				Audio Task chuyển sang BLOCKED  
						|  
						v  
				Không chiếm CPU  
						|  
				Button Task gửi command  
						|  
						v  
				Audio Task chuyển sang READY  
		  
* Nếu không blocking, task có thể phải kiểm tra liên tục , đó là polling , gây lãng phí CPU

* Blocking giúp giảm tải CPU, cho phép task khác chạy,tạo kiến trúc event-driven

### Delay theo tick

* FreeRTOS quản lý thời gian theo đơn vị tick

* Khi sử dụng delay, số ms được chuyển sang tick của hệ điều hành

			Thời gian mong muốn
					|  
					v  
			Chuyển thành số tick  
					|  
					v  
			Task chuyển sang BLOCKED  
					|						  
					v  
			Đủ tick thì quay về READY

* Trong dự án:

	* Button Task delay giữa các lần quét phím
	
	* Visualizer Task delay giữa các lần cập nhật LED
	
	* LED Task Delay để tạo chu kỳ nháy
	
	* Display Task dùng delay khi hiển thị màn hình khởi động


* Phân biệt Busy delay và RTOS delay

	* Busy delay
	
		* CPU chạy vòng lặp chờ
		
			* Task vẫn giữ CPU
			* Task khác khó được chạy
			* Lãng phí thời gian xử lý
			
	* RTOS delay
	
		* Task chuyển sang blocked
		
			*  Task không giữ CPU
			*  Scheduler chạy task khác
			*  Đủ thời gian thì task được đánh thức      


### Luồng giao tiếp giữa các task

					Button Task  
						|  
						| Audio command  
						v  
					Audio Queue  
						|  
						v  
					Audio Task  
						|  
						| Player state  
						v  
					Display Queue  
						|  
						v  
					Display Task
  
## Kiến trúc phần mềm

### Modular desing

*  Modular design là chia phần mềm thành các module có trách nhiệm và interface rõ ràng.

*  Mỗi module che giấu chi tiết triển khai bên trong.

### Producer-consumer

* Producer là thành phần tạo dữ liệu của sự kiện

* Consumer là thành phần nhận và xử lý dữ liệu đó

* Producer-consumer thứ nhất:

	* Button Task 	= Producer
	* Audio Queue   = Buffer trung gian
	* Audio Task 		 = Consumer

* Producer-consumer thứ hai:

	* Audio Task = Producer
	* Display Queue = Buffer trung gian
	* Display Task = Consumer

* Vai trò của queue:

	* Producer và consumer không bắt buộc phải chạy cùng thời điểm nhờ vậy 2 task đọc lập về timing

			Producer tạo dữ liệu âm thanh
			Queue lưu tạm
			Consumer xử lý khi được scheduler chạy     

### Event-Driven

* Event-Driven là hệ thông chủ yếu xử lý khi có sự kiện, thay vì tất cả chức năng liên tục kiểm tra trạng thái

* Các event trong dự án gồm:

	* Người dùng nhấn nút
	* Có command mới trong Audio Queue
	* Trạng thái bài hát thay đổi
	* Volume thay đổi
	* Có dữ liệu mới cần cập nhật màn hình

* Luồng event:

			  Người dùng nhấn nút  
						|  
						v  
			Button Task tạo command event  
						|  
						v  
			Audio Task xử lý event  
						|  
						v  
			Trạng thái audio thay đổi  
						|  
						v  
			Audio Task tạo display event  
						|  
						v  
			Display Task cập nhật giao diện


### Single ownership

* Single ownership là một tài nguyên hoặc trạng thái quan trọng chỉ được một task sở hữu và thay đổi

* Audio Task sở hữu trạng thái player, các task khác không trực tiếp sửa giá trị này

* Display Task sở hữu màn hình

* Visualizer Task sở hữu quá trình cập nhật LED visualizer

### State-based processing

* Hệ thống không chỉ thực hiện các hàm rời rạc mà còn duy trì trạng thái hiện tại

* Trạng thái audio gồm:

	*  Bài hiện tại
    *  Đang phát hay tạm dừng
    *  Volume hiện tại
    *  Tổng số bài


## Xử lý tín hiệu cơ bản

### ADC Sampling

* Tín hiệu audio là tín hiệu analog biến đổi liên tục theo thời gian.

* MCU không xử lý trực tiếp tín hiệu analog mà phải dùng ADC để chuyển nó thành các giá trị số.

		 Tín hiệu audio analog  
					|  
					v  
				   ADC  
					|  
					v  
		Chuỗi giá trị số theo thời gian  

### Tìm giá trị min và max

* ADC_max = max(x1,x2,...xN)
* ADC_min = min(x1,x2,...,xN)

	* x_i : từng sample ADC 

* Nếu tín hiệu dao động mạnh 

	* Giá trị max sẽ cao
	* Giá trị min sẽ thấp
	* Khoảng cách giữa max và min lớn

* Nếu tín hiệu nhỏ hoặc gần im lặng:

	* Max và min gần nhau
	* Khoảng cách nó

* Minh họa:

	* Tín hiệu nhỏ: 

			Samples: 2010, 2020, 1995, 2015
			Max = 2020
			Min = 1995

	* Tín hiệu lớn:

			Samples: 1500, 2500, 1800, 2300
			Max = 2500
			Min = 1500

### Tính biên độ peak-to-peak

* Biên độ peak-to-peak được tính bằng:

	* App = ADCmax - ADCmin

* VD:

	* Tín hiệu nhỏ:

				App = 2020 - 1995 = 25

	* Tín hiệu lớn:

				App = 2500 - 1500 = 1000

### Làm mượt bằng cơ chế tăng nhanh, giảm chậm

*  Nếu dùng trực tiếp biên độ vừa đo để điều khiển LED, số LED có thể thay đổi rất nhanh:

				2 LED -> 7 LED -> 1 LED -> 6 LED -> 0 LED

		* Kết quả nhìn sẽ bị giật và nhấp nháy

*  Dự án dùng cơ chế:

	* Nếu biên độ mới lớn hơn mức đang hiển thị thì cập nhật ngay
	* Nếu biên độ mới nhỏ hơn thì không giảm ngay về giá trị mới.
	* Mức hiển thị chỉ giảm từng bước nhỏ.

### Chuyển biên độ thành số lượng LED sáng

*  Sau khi có biên độ đã làm mượt, hệ thống ánh xạ nó sang số LED:

				Nled = Adisplay x Ntotal / Areference

	* Adisplay: biên độ sau làm mượt
	* Ntotal: tổng số LED
	* Areference: mức biên độ được coi là full-scale  
