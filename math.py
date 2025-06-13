import mmap
import struct
import time
import numpy as np

class SharedMemoryReader:
    def __init__(self):
        self.shm_fd = None
        self.shared_memory = None
        
    def connect(self):
        try:
            import os
            self.shm_fd = os.open("/dev/shm/trading_shm", os.O_RDWR)
            self.shared_memory = mmap.mmap(self.shm_fd, 820)  # sizeof(SharedData)
            
            # Signal that Python is ready
            self.shared_memory.seek(808)  # offset for python_ready
            self.shared_memory.write(struct.pack('?', True))
            self.shared_memory.flush()
            
            print("Connected to shared memory")
            return True
        except Exception as e:
            print(f"Failed to connect to shared memory: {e}")
            return False
    
    def read_market_data(self):
        if not self.shared_memory:
            return None, 0
            
        self.shared_memory.seek(0)
        data_bytes = self.shared_memory.read(800)  # 100 * 8 bytes for doubles
        
        self.shared_memory.seek(800)
        count_bytes = self.shared_memory.read(4)
        count = struct.unpack('i', count_bytes)[0]
        
        if count > 0:
            data = struct.unpack(f'{count}d', data_bytes[:count*8])
            return np.array(data), count
        
        return None, 0
    
    def process_data(self, data):
        if data is not None and len(data) > 0:
            mean_val = np.mean(data)
            std_val = np.std(data)
            print(f"Market Data Analysis - Mean: {mean_val:.2f}, Std: {std_val:.2f}")
            return mean_val, std_val
        return None, None
    
    def cleanup(self):
        if self.shared_memory:
            self.shared_memory.close()
        if self.shm_fd:
            import os
            os.close(self.shm_fd)

def main():
    reader = SharedMemoryReader()
    
    if not reader.connect():
        return 1
    
    print("Python math processor started")
    
    try:
        while True:
            data, count = reader.read_market_data()
            if count > 0:
                reader.process_data(data)
            
            time.sleep(1)
    except KeyboardInterrupt:
        print("Python processor shutting down")
    finally:
        reader.cleanup()
    
    return 0

if __name__ == "__main__":
    exit(main())
