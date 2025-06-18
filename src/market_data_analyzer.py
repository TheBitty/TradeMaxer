#!/usr/bin/env python3

import json
import sqlite3
import numpy as np
import pandas as pd
from datetime import datetime, timedelta
import os
import sys
import time
from typing import List, Dict, Tuple
import signal

# Import our neural network
from stock_ranking_nn import StockRanker

class MarketDataAnalyzer:
    def __init__(self, db_path="trading_system.db", pipe_base="/tmp/trading_system_pipe"):
        self.db_path = db_path
        self.pipe_to_python = pipe_base + "_to_python"
        self.pipe_to_cpp = pipe_base + "_to_cpp"
        self.running = True
        self.ranker = None
        
        # Set up signal handler for graceful shutdown
        signal.signal(signal.SIGTERM, self.signal_handler)
        signal.signal(signal.SIGINT, self.signal_handler)
        
    def signal_handler(self, signum, frame):
        print("Received shutdown signal")
        self.running = False
        
    def connect_database(self):
        """Connect to SQLite database"""
        self.conn = sqlite3.connect(self.db_path)
        self.cursor = self.conn.cursor()
        
    def fetch_market_data(self, symbol: str, days: int = 30) -> pd.DataFrame:
        """Fetch market data from database"""
        end_date = datetime.now()
        start_date = end_date - timedelta(days=days)
        
        query = """
        SELECT symbol, open, high, low, close, volume, timestamp
        FROM market_data
        WHERE symbol = ? AND timestamp >= ? AND timestamp <= ?
        ORDER BY timestamp DESC
        """
        
        df = pd.read_sql_query(query, self.conn, 
                              params=(symbol, start_date.strftime('%Y-%m-%d'), 
                                     end_date.strftime('%Y-%m-%d')))
        
        if not df.empty:
            df['timestamp'] = pd.to_datetime(df['timestamp'])
            df.set_index('timestamp', inplace=True)
        
        return df
    
    def calculate_technical_indicators(self, df: pd.DataFrame) -> pd.DataFrame:
        """Calculate technical indicators for the data"""
        if df.empty:
            return df
        
        # Simple Moving Averages
        df['SMA_5'] = df['close'].rolling(window=5).mean()
        df['SMA_20'] = df['close'].rolling(window=20).mean()
        
        # Exponential Moving Averages
        df['EMA_12'] = df['close'].ewm(span=12, adjust=False).mean()
        df['EMA_26'] = df['close'].ewm(span=26, adjust=False).mean()
        
        # MACD
        df['MACD'] = df['EMA_12'] - df['EMA_26']
        df['MACD_signal'] = df['MACD'].ewm(span=9, adjust=False).mean()
        
        # RSI
        delta = df['close'].diff()
        gain = (delta.where(delta > 0, 0)).rolling(window=14).mean()
        loss = (-delta.where(delta < 0, 0)).rolling(window=14).mean()
        rs = gain / loss
        df['RSI'] = 100 - (100 / (1 + rs))
        
        # Bollinger Bands
        df['BB_middle'] = df['close'].rolling(window=20).mean()
        bb_std = df['close'].rolling(window=20).std()
        df['BB_upper'] = df['BB_middle'] + (bb_std * 2)
        df['BB_lower'] = df['BB_middle'] - (bb_std * 2)
        
        # Volume indicators
        df['Volume_SMA'] = df['volume'].rolling(window=20).mean()
        df['Volume_ratio'] = df['volume'] / df['Volume_SMA']
        
        # Price changes
        df['Price_change'] = df['close'].pct_change()
        df['Price_change_5d'] = df['close'].pct_change(periods=5)
        
        return df
    
    def prepare_features(self, df: pd.DataFrame) -> np.ndarray:
        """Prepare features for neural network"""
        if df.empty or len(df) < 30:  # Need enough data for indicators
            return np.array([])
        
        # Select latest row with all indicators calculated
        latest = df.iloc[-1]
        
        features = []
        
        # Price-based features
        features.append(latest['close'] / latest['SMA_20'] if latest['SMA_20'] > 0 else 1.0)
        features.append(latest['close'] / latest['SMA_5'] if latest['SMA_5'] > 0 else 1.0)
        features.append((latest['close'] - latest['low']) / (latest['high'] - latest['low']) if latest['high'] > latest['low'] else 0.5)
        
        # Technical indicators
        features.append(latest['RSI'] / 100.0 if not pd.isna(latest['RSI']) else 0.5)
        features.append(latest['MACD'] / latest['close'] if latest['close'] > 0 else 0.0)
        features.append((latest['close'] - latest['BB_lower']) / (latest['BB_upper'] - latest['BB_lower']) if latest['BB_upper'] > latest['BB_lower'] else 0.5)
        
        # Volume features
        features.append(latest['Volume_ratio'] if not pd.isna(latest['Volume_ratio']) else 1.0)
        
        # Momentum features
        features.append(latest['Price_change'] if not pd.isna(latest['Price_change']) else 0.0)
        features.append(latest['Price_change_5d'] if not pd.isna(latest['Price_change_5d']) else 0.0)
        
        # Volatility
        volatility = df['Price_change'].std()
        features.append(volatility if not pd.isna(volatility) else 0.0)
        
        return np.array(features)
    
    def analyze_symbol(self, symbol: str) -> Dict:
        """Analyze a single symbol and generate trading signal"""
        # Fetch market data
        df = self.fetch_market_data(symbol)
        
        if df.empty:
            return {
                'symbol': symbol,
                'action': 'HOLD',
                'confidence': 0.0,
                'reason': 'Insufficient data'
            }
        
        # Calculate indicators
        df = self.calculate_technical_indicators(df)
        
        # Prepare features
        features = self.prepare_features(df)
        
        if features.size == 0:
            return {
                'symbol': symbol,
                'action': 'HOLD',
                'confidence': 0.0,
                'reason': 'Insufficient data for analysis'
            }
        
        # Initialize ranker if needed
        if self.ranker is None:
            self.ranker = StockRanker(input_features=len(features))
        
        # Get prediction (for now, using rule-based logic until we train on real data)
        latest = df.iloc[-1]
        
        # Simple trading rules
        action = 'HOLD'
        confidence = 0.5
        reasons = []
        
        # RSI signals
        if latest['RSI'] < 30:
            action = 'BUY'
            confidence += 0.2
            reasons.append('RSI oversold')
        elif latest['RSI'] > 70:
            action = 'SELL'
            confidence += 0.2
            reasons.append('RSI overbought')
        
        # MACD signals
        if latest['MACD'] > latest['MACD_signal'] and latest['MACD'] > 0:
            if action != 'SELL':
                action = 'BUY'
                confidence += 0.15
                reasons.append('MACD bullish crossover')
        elif latest['MACD'] < latest['MACD_signal'] and latest['MACD'] < 0:
            if action != 'BUY':
                action = 'SELL'
                confidence += 0.15
                reasons.append('MACD bearish crossover')
        
        # Moving average signals
        if latest['close'] > latest['SMA_20'] and latest['SMA_5'] > latest['SMA_20']:
            if action != 'SELL':
                action = 'BUY'
                confidence += 0.1
                reasons.append('Price above moving averages')
        elif latest['close'] < latest['SMA_20'] and latest['SMA_5'] < latest['SMA_20']:
            if action != 'BUY':
                action = 'SELL'
                confidence += 0.1
                reasons.append('Price below moving averages')
        
        # Bollinger Band signals
        if latest['close'] < latest['BB_lower']:
            if action != 'SELL':
                action = 'BUY'
                confidence += 0.1
                reasons.append('Price at lower Bollinger Band')
        elif latest['close'] > latest['BB_upper']:
            if action != 'BUY':
                action = 'SELL'
                confidence += 0.1
                reasons.append('Price at upper Bollinger Band')
        
        # Normalize confidence
        confidence = min(confidence, 0.95)
        
        return {
            'symbol': symbol,
            'action': action,
            'confidence': confidence,
            'suggested_position_size': self.calculate_position_size(confidence, volatility),
            'reasons': reasons,
            'current_price': float(latest['close']),
            'technical_data': {
                'rsi': float(latest['RSI']) if not pd.isna(latest['RSI']) else None,
                'macd': float(latest['MACD']) if not pd.isna(latest['MACD']) else None,
                'volume_ratio': float(latest['Volume_ratio']) if not pd.isna(latest['Volume_ratio']) else None
            }
        }
    
    def calculate_position_size(self, confidence: float, volatility: float) -> float:
        """Calculate suggested position size based on confidence and volatility"""
        base_size = 1000.0  # Base position size in USD
        
        # Adjust for confidence
        size = base_size * confidence
        
        # Adjust for volatility (inverse relationship)
        if volatility > 0:
            volatility_factor = min(0.02 / volatility, 2.0)  # Cap at 2x
            size *= volatility_factor
        
        return round(size, 2)
    
    def save_signal(self, signal: Dict):
        """Save trading signal to database"""
        query = """
        INSERT INTO trading_signals (symbol, confidence, action, suggested_position_size, timestamp)
        VALUES (?, ?, ?, ?, ?)
        """
        
        self.cursor.execute(query, (
            signal['symbol'],
            signal['confidence'],
            signal['action'],
            signal['suggested_position_size'],
            datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        ))
        self.conn.commit()
    
    def handle_message(self, message: str) -> str:
        """Handle incoming messages from C++"""
        try:
            data = json.loads(message)
            command = data.get('command')
            
            if command == 'analyze':
                symbol = data.get('symbol', 'BTC')
                result = self.analyze_symbol(symbol)
                self.save_signal(result)
                return json.dumps(result)
                
            elif command == 'batch_analyze':
                symbols = data.get('symbols', [])
                results = []
                for symbol in symbols:
                    result = self.analyze_symbol(symbol)
                    self.save_signal(result)
                    results.append(result)
                return json.dumps({'results': results})
                
            elif command == 'get_positions':
                # Return current recommended positions
                query = """
                SELECT symbol, action, confidence, suggested_position_size, timestamp
                FROM trading_signals
                WHERE timestamp > datetime('now', '-1 hour')
                ORDER BY confidence DESC
                LIMIT 10
                """
                self.cursor.execute(query)
                positions = []
                for row in self.cursor.fetchall():
                    positions.append({
                        'symbol': row[0],
                        'action': row[1],
                        'confidence': row[2],
                        'suggested_position_size': row[3],
                        'timestamp': row[4]
                    })
                return json.dumps({'positions': positions})
                
            else:
                return json.dumps({'error': 'Unknown command'})
                
        except Exception as e:
            return json.dumps({'error': str(e)})
    
    def run(self):
        """Main loop for the analyzer"""
        print("Market Data Analyzer starting...")
        
        # Connect to database
        self.connect_database()
        
        # Wait for pipes to be created
        while not os.path.exists(self.pipe_to_python) or not os.path.exists(self.pipe_to_cpp):
            print("Waiting for IPC pipes...")
            time.sleep(1)
            if not self.running:
                return
        
        print("IPC pipes found, opening connections...")
        
        # Open pipes
        pipe_in = open(self.pipe_to_python, 'r')
        pipe_out = open(self.pipe_to_cpp, 'w')
        
        print("Market Data Analyzer ready")
        
        try:
            while self.running:
                # Read message from C++
                line = pipe_in.readline().strip()
                if line:
                    print(f"Received: {line}")
                    response = self.handle_message(line)
                    print(f"Sending: {response}")
                    pipe_out.write(response + '\n')
                    pipe_out.flush()
                else:
                    time.sleep(0.1)
                    
        except KeyboardInterrupt:
            print("Shutting down...")
        finally:
            pipe_in.close()
            pipe_out.close()
            self.conn.close()
            print("Market Data Analyzer stopped")

if __name__ == "__main__":
    analyzer = MarketDataAnalyzer()
    analyzer.run()