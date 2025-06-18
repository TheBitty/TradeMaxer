import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
from typing import List, Tuple, Dict
import pandas as pd
from sklearn.preprocessing import StandardScaler


class StockRankingNN(nn.Module):
    def __init__(self, input_features: int, hidden_sizes: List[int] = [64, 32, 16]):
        super(StockRankingNN, self).__init__()
        
        layers = []
        prev_size = input_features
        
        for hidden_size in hidden_sizes:
            layers.extend([
                nn.Linear(prev_size, hidden_size),
                nn.ReLU(),
                nn.BatchNorm1d(hidden_size),
                nn.Dropout(0.2)
            ])
            prev_size = hidden_size
        
        layers.append(nn.Linear(prev_size, 1))
        
        self.network = nn.Sequential(*layers)
    
    def forward(self, x):
        return self.network(x)


class StockDataset(Dataset):
    def __init__(self, features: np.ndarray, labels: np.ndarray):
        self.features = torch.FloatTensor(features)
        self.labels = torch.FloatTensor(labels)
    
    def __len__(self):
        return len(self.features)
    
    def __getitem__(self, idx):
        return self.features[idx], self.labels[idx]


class StockRanker:
    def __init__(self, input_features: int, hidden_sizes: List[int] = [64, 32, 16]):
        self.model = StockRankingNN(input_features, hidden_sizes)
        self.scaler = StandardScaler()
        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        self.model.to(self.device)
    
    def preprocess_features(self, stock_data: pd.DataFrame, feature_columns: List[str]) -> np.ndarray:
        features = stock_data[feature_columns].values
        
        features = np.nan_to_num(features, nan=0.0)
        
        if hasattr(self.scaler, 'mean_'):
            features = self.scaler.transform(features)
        else:
            features = self.scaler.fit_transform(features)
        
        return features
    
    def pairwise_ranking_loss(self, scores: torch.Tensor, labels: torch.Tensor, margin: float = 1.0):
        n = len(scores)
        losses = []
        
        for i in range(n):
            for j in range(i + 1, n):
                if labels[i] > labels[j]:
                    loss = torch.relu(margin - (scores[i] - scores[j]))
                elif labels[i] < labels[j]:
                    loss = torch.relu(margin - (scores[j] - scores[i]))
                else:
                    continue
                losses.append(loss)
        
        return torch.mean(torch.stack(losses)) if losses else torch.tensor(0.0)
    
    def train(self, 
              train_features: np.ndarray, 
              train_labels: np.ndarray,
              val_features: np.ndarray = None,
              val_labels: np.ndarray = None,
              epochs: int = 100,
              batch_size: int = 32,
              learning_rate: float = 0.001):
        
        train_dataset = StockDataset(train_features, train_labels)
        train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
        
        optimizer = optim.Adam(self.model.parameters(), lr=learning_rate)
        
        for epoch in range(epochs):
            self.model.train()
            total_loss = 0
            
            for batch_features, batch_labels in train_loader:
                batch_features = batch_features.to(self.device)
                batch_labels = batch_labels.to(self.device)
                
                optimizer.zero_grad()
                
                scores = self.model(batch_features).squeeze()
                
                loss = self.pairwise_ranking_loss(scores, batch_labels)
                
                loss.backward()
                optimizer.step()
                
                total_loss += loss.item()
            
            avg_loss = total_loss / len(train_loader)
            
            if epoch % 10 == 0:
                print(f"Epoch {epoch}/{epochs}, Loss: {avg_loss:.4f}")
                
                if val_features is not None and val_labels is not None:
                    val_score = self.evaluate(val_features, val_labels)
                    print(f"Validation Ranking Score: {val_score:.4f}")
    
    def evaluate(self, features: np.ndarray, labels: np.ndarray) -> float:
        self.model.eval()
        
        with torch.no_grad():
            features_tensor = torch.FloatTensor(features).to(self.device)
            scores = self.model(features_tensor).cpu().numpy().squeeze()
        
        predicted_ranks = np.argsort(-scores)
        true_ranks = np.argsort(-labels)
        
        correct = 0
        total = 0
        
        for i in range(len(predicted_ranks)):
            for j in range(i + 1, len(predicted_ranks)):
                if labels[predicted_ranks[i]] != labels[predicted_ranks[j]]:
                    total += 1
                    if (labels[predicted_ranks[i]] > labels[predicted_ranks[j]]):
                        correct += 1
        
        return correct / total if total > 0 else 0.0
    
    def predict_rankings(self, features: np.ndarray) -> np.ndarray:
        self.model.eval()
        
        with torch.no_grad():
            features_tensor = torch.FloatTensor(features).to(self.device)
            scores = self.model(features_tensor).cpu().numpy().squeeze()
        
        rankings = np.argsort(-scores)
        return rankings, scores


def create_mock_stock_data(n_stocks: int = 100, n_features: int = 10) -> Tuple[pd.DataFrame, np.ndarray]:
    np.random.seed(42)
    
    feature_names = [f"feature_{i}" for i in range(n_features)]
    
    features = np.random.randn(n_stocks, n_features)
    
    true_weights = np.random.randn(n_features)
    scores = features @ true_weights + np.random.randn(n_stocks) * 0.1
    
    labels = (scores - scores.min()) / (scores.max() - scores.min())
    
    stock_data = pd.DataFrame(features, columns=feature_names)
    stock_data['ticker'] = [f"STOCK_{i}" for i in range(n_stocks)]
    
    return stock_data, labels


if __name__ == "__main__":
    stock_data, labels = create_mock_stock_data(n_stocks=1000, n_features=10)
    
    feature_columns = [col for col in stock_data.columns if col.startswith('feature_')]
    
    train_size = int(0.8 * len(stock_data))
    train_data = stock_data[:train_size]
    train_labels = labels[:train_size]
    val_data = stock_data[train_size:]
    val_labels = labels[train_size:]
    
    ranker = StockRanker(input_features=len(feature_columns), hidden_sizes=[64, 32, 16])
    
    train_features = ranker.preprocess_features(train_data, feature_columns)
    val_features = ranker.preprocess_features(val_data, feature_columns)
    
    print("Training Stock Ranking Neural Network...")
    ranker.train(
        train_features, 
        train_labels,
        val_features,
        val_labels,
        epochs=50,
        batch_size=32,
        learning_rate=0.001
    )
    
    print("\nPredicting rankings on validation set...")
    rankings, scores = ranker.predict_rankings(val_features)
    
    print("\nTop 10 ranked stocks:")
    for i in range(min(10, len(rankings))):
        stock_idx = rankings[i]
        print(f"{i+1}. {val_data.iloc[stock_idx]['ticker']} - Score: {scores[stock_idx]:.4f}")